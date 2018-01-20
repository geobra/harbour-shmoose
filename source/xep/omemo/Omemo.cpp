/*
 *
 * This is a port of
 * https://github.com/gkdr/lurch/blob/master/src/lurch.c
 *
 * Concept is to stay as much as possible at the original functions to make change tracking easier
 *
 */

#include "Omemo.h"
#include "System.h"

#include <gcrypt.h>
#include <mxml.h>

#include <QDir>
#include <QDomDocument>
#include <QDebug>

#define LURCH_DB_SUFFIX "_db.sqlite"
#define LURCH_DB_NAME_OMEMO "omemo"
#define LURCH_DB_NAME_AXC "axc"

Omemo::Omemo(QObject *parent) : QObject(parent),
    client_(NULL), actualBareJid(""), uninstall(0)
{
    // create omemo path if needed
    QString omemoLocation = System::getOmemoPath();
    QDir dir(omemoLocation);

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // init omemo crypto
    omemo_default_crypto_init();
}

Omemo::~Omemo()
{
    omemo_default_crypto_teardown();
}

void Omemo::setupWithClient(Swift::Client *client)
{
    if (client != NULL)
    {
        client_ = client;

        // FIXME connect msg send, msg receive, pep device list update
        client_->onMessageReceived.connect(boost::bind(&Omemo::handleMessageReceived, this, _1));

        // FIXME for device list updates
        // lurch_pep_devicelist_event_handler

        // FIXME catches to be sent messages and encrypts
        //lurch_xml_sent_cb => lurch_message_encrypt_im || lurch_message_encrypt_groupchat

        // FIXME handles presence and message decryption
        // lurch_xml_received_cb

        // request own device list
        // similar to lurch_account_connect_cb()
        requestDeviceList(client_->getJID().toBare());
    }
}

void Omemo::requestDeviceList(const Swift::JID& jid)
{
    char * dl_ns = (char *) 0;

    int ret_val = omemo_devicelist_get_pep_node_name(&dl_ns);
    if (ret_val)
    {
        qDebug() << "failed to get devicelist pep node name";
    }
    else
    {
        actualBareJid = jid.toBare();

        // gen the payload
        const std::string deviceListRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(dl_ns) + "'/></pubsub>";

        Swift::RawRequest::ref requestDeviceList = Swift::RawRequest::create(Swift::IQ::Get, jid.toBare(), deviceListRequestXml, client_->getIQRouter());
        requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1));

        requestDeviceList->send();

        free(dl_ns);
    }
}

void Omemo::handleDeviceListResponse(const std::string& str)
{
    std::cout << "OMEMO: handle Response: " << str;

    // handle own device list
    if (actualBareJid.toString() == client_->getJID().toBare().toString())
    {
        lurch_pep_own_devicelist_request_handler(str);
    }
}

void Omemo::publishPepRawXml(const Swift::JID& jid, const QString& rawXml)
{
    // gen the payload
    const std::string pubsubXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'>" + rawXml.toStdString() + "</pubsub>";

    Swift::RawRequest::ref publishPep = Swift::RawRequest::create(Swift::IQ::Set,
                                                                            jid.toBare(),
                                                                            pubsubXml,
                                                                            client_->getIQRouter());
    // FIXME create responder to catch the errors
    //requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1));

    publishPep->send();
}


void Omemo::handleMessageReceived(Swift::Message::ref message)
{
    std::cout << "OMEMO: handleMessageReceived: jid: " << message->getFrom() << ", bare: " << message->getFrom().toBare().toString() << ", resource: " << message->getFrom().getResource() << std::endl;
}

/**
 * A JabberPEPHandler function.
 * Is used to handle the own devicelist and also perform install-time functions.
 */
void Omemo::lurch_pep_own_devicelist_request_handler(const std::string &items_p) {

#if 0
 char * response = "<pubsub xmlns=\"http://jabber.org/protocol/pubsub\"> \
  <items node=\"eu.siacs.conversations.axolotl.devicelist\"> \
   <item id=\"COFFEEBABE\"> \
    <list xmlns=\"eu.siacs.conversations.axolotl\"> \
     <device id=\"1234567890\"></device> \
    </list> \
   </item> \
  </items> \
 </pubsub>";

         char * strippedXmlCStr = "<items node=\"eu.siacs.conversations.axolotl.devicelist\"> \
           <item id=\"COFFEEBABE\"> \
            <list xmlns=\"eu.siacs.conversations.axolotl\"> \
             <device id=\"1234567890\"></device> \
            </list> \
           </item> \
          </items>";
#endif

    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    int install = 0;
    axc_context * axc_ctx_p = nullptr;
    uint32_t own_id = 0;
    int needs_publishing = 1;
    omemo_devicelist * dl_p = nullptr;
    char * dl_xml = nullptr;

    std::string bareJid = client_->getJID().toBare().toString();
    QByteArray JidArray = QString::fromStdString(bareJid).toLocal8Bit();
    char* uname = JidArray.data();

    //install = (purple_account_get_bool(acc_p, LURCH_ACC_SETTING_INITIALIZED, FALSE)) ? 0 : 1;
    // FIXME
    install = 1; // FIXME set to 0 if already installed

    if (install && !uninstall) {
        purple_debug_info("lurch", "%s: %s\n", __func__, "preparing installation...");
        ret_val = lurch_axc_prepare(uname);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to prepare axc");
            goto cleanup;
        }
        purple_debug_info("lurch", "%s: %s\n", __func__, "...done");
    }

    ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to init axc ctx");
        goto cleanup;
    }
    ret_val = axc_get_device_id(axc_ctx_p, &own_id);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get own id");
        goto cleanup;
    }

    if (items_p.empty()) { // FIXME this function wont be called on an empty device list
    //if (true) {
        purple_debug_info("lurch", "%s: %s\n", __func__, "no devicelist yet, creating it");
        ret_val = omemo_devicelist_create(uname, &dl_p);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to create devicelist");
            goto cleanup;
        }
        ret_val = omemo_devicelist_add(dl_p, own_id);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to add own id %i to devicelist", own_id);
            goto cleanup;
        }
    } else {
        purple_debug_info("lurch", "%s: %s\n", __func__, "comparing received devicelist with cached one");
        QString strippedXml = getItemsChildFromPubSubXml(QString::fromStdString(items_p));

        QByteArray xmlArray = strippedXml.toLocal8Bit();
        char* strippedXmlCStr = xmlArray.data();

        fprintf(stderr, "items: %s\n", strippedXmlCStr);
#if 0
        mxml_node_t * items_node_p = mxmlLoadString(nullptr, strippedXmlCStr, MXML_NO_CALLBACK);
        if (items_node_p == NULL)
            qDebug() << "items is NULL";
        ret_val = strncmp(mxmlGetElement(items_node_p), "items", strlen("items"));
        qDebug() << "items: " << ret_val;

        mxml_node_t * item_node_p = mxmlGetFirstChild(items_node_p);
        //mxml_node_t * item_node_p = mxmlGetNextSibling(items_node_p);
        if (item_node_p == NULL)
            qDebug() << "item is NULL";
        ret_val = strncmp(mxmlGetElement(item_node_p), "item", strlen("item"));
        qDebug() << "item: " << ret_val;
#endif
        // FIXME crashes in this call
        // no answer from libomemo owner until now
        // TODO improve own fix an make pull request
        ret_val = omemo_devicelist_import(strippedXmlCStr, uname, &dl_p);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to import received devicelist");
            goto cleanup;
        }

        ret_val = omemo_devicelist_contains_id(dl_p, own_id);
        if (ret_val == 1) {
            purple_debug_info("lurch", "%s: %s\n", __func__, "own id was already contained in received devicelist, doing nothing");
            needs_publishing = 0;
        } else if (ret_val == 0) {
            if (!uninstall) {
                purple_debug_info("lurch", "%s: %s\n", __func__, "own id was missing, adding it");
                ret_val = omemo_devicelist_add(dl_p, own_id);
                if (ret_val) {
                    err_msg_dbg = g_strdup_printf("failed to add own id %i to devicelist", own_id);
                    goto cleanup;
                }
            } else {
                needs_publishing = 0;
            }

        } else {
            err_msg_dbg = g_strdup_printf("failed to look up if the devicelist contains the own id");
            goto cleanup;
        }
    }

    if (needs_publishing) {
        purple_debug_info("lurch", "%s: %s\n", __func__, "devicelist needs publishing...");
        ret_val = omemo_devicelist_export(dl_p, &dl_xml);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to export new devicelist");
            goto cleanup;
        }

        publishPepRawXml(client_->getJID(), QString(dl_xml));

        //std::cout << "pubish pep (response own device list): " << dl_xml;
        /*
         * <publish node="eu.siacs.conversations.axolotl.devicelist">
         *   <item>
         *     <list xmlns="eu.siacs.conversations.axolotl">
         *        <device id="1394567069" />
         *     </list>
         *    </item>
         * </publish>
         */

        purple_debug_info("lurch", "%s: \n%s:\n", __func__, "...done");
    }

    ret_val = lurch_bundle_publish_own(/*js_p*/);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to publish own bundle");
        goto cleanup;
    }

    if (install && !uninstall) {
        //purple_account_set_bool(acc_p, LURCH_ACC_SETTING_INITIALIZED, TRUE);
        // FIXME
    }

    ret_val = lurch_devicelist_process(uname, dl_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to process the devicelist");
        goto cleanup;
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }

    axc_context_destroy_all(axc_ctx_p);
    omemo_devicelist_destroy(dl_p);
    free(dl_xml);
}

/**
 * Does the first-time install of the axc DB.
 * As specified in OMEMO, it checks if the generated device ID already exists.
 * Therefore, it should be called at a point in time when other entries exist.
 *
 * @param uname The username.
 * @return 0 on success, negative on error.
 */
int Omemo::lurch_axc_prepare(char * uname) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    axc_context * axc_ctx_p = nullptr;
    uint32_t device_id = 0;
    char * db_fn_omemo = nullptr;

    ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get init axc ctx");
        goto cleanup;
    }

    ret_val = axc_get_device_id(axc_ctx_p, &device_id);
    if (!ret_val) {
        // already installed
        goto cleanup;
    }

    db_fn_omemo = lurch_uname_get_db_fn(uname, LURCH_DB_NAME_OMEMO);

    while (1) {
        ret_val = axc_install(axc_ctx_p);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to install axc");
            goto cleanup;
        }

        ret_val = axc_get_device_id(axc_ctx_p, &device_id);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to get own device id");
            goto cleanup;
        }

        ret_val = omemo_storage_global_device_id_exists(device_id, db_fn_omemo);
        if (ret_val == 1)  {
            ret_val = axc_db_init_status_set(AXC_DB_NEEDS_ROLLBACK, axc_ctx_p);
            if (ret_val) {
                err_msg_dbg = g_strdup_printf("failed to set axc db to rollback");
                goto cleanup;
            }
        } else if (ret_val < 0) {
            err_msg_dbg = g_strdup_printf("failed to access the db %s", db_fn_omemo);
            goto cleanup;
        } else {
            break;
        }
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }
    axc_context_destroy_all(axc_ctx_p);
    free(db_fn_omemo);

    return ret_val;
}


/**
 * Creates and initializes the axc context.
 *
 * @param uname The username.
 * @param ctx_pp Will point to an initialized axc context on success.
 * @return 0 on success, negative on error.
 */
int Omemo::lurch_axc_get_init_ctx(char * uname, axc_context ** ctx_pp) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    axc_context * ctx_p = nullptr;
    char * db_fn = nullptr;

    ret_val = axc_context_create(&ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to create axc context");
        goto cleanup;
    }

    db_fn = lurch_uname_get_db_fn(uname, LURCH_DB_NAME_AXC);
    ret_val = axc_context_set_db_fn(ctx_p, db_fn, strlen(db_fn));
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to set axc db filename");
        goto cleanup;
    }

    ret_val = axc_init(ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to init axc context");
        goto cleanup;
    }

    *ctx_pp = ctx_p;

cleanup:
    if (ret_val) {
        axc_context_destroy_all(ctx_p);
    }
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }

    free (db_fn);
    return ret_val;
}

/**
 * Returns the db name, has to be g_free()d.
 *
 * @param uname The username.
 * @param which Either LURCH_DB_NAME_OMEMO or LURCH_DB_NAME_AXC
 * @return The path string.
 */
char * Omemo::lurch_uname_get_db_fn(const char * uname, char * which)
{
    std::string omemoPath = System::getOmemoPath().toStdString();
    return g_strconcat(omemoPath.c_str(), "/", uname, "_", which, LURCH_DB_SUFFIX, NULL);
}

/**
 * Collects the information needed for a bundle and publishes it.
 *
 * @param uname The username.
 * @param js_p Pointer to the connection to use for publishing.
 */
int Omemo::lurch_bundle_publish_own(/*JabberStream * js_p*/) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    axc_context * axc_ctx_p = nullptr;
    axc_bundle * axcbundle_p = nullptr;
    omemo_bundle * omemobundle_p = nullptr;
    axc_buf * curr_buf_p = nullptr;
    axc_buf_list_item * next_p = nullptr;
    char * bundle_xml = nullptr;
    //xmlnode * publish_node_bundle_p = nullptr;

    std::string bareJid = client_->getJID().toBare().toString();
    QByteArray barJidArray = QString::fromStdString(bareJid).toLocal8Bit();
    char* uname = barJidArray.data();


    ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to init axc ctx");
        goto cleanup;
    }

    ret_val = axc_bundle_collect(AXC_PRE_KEYS_AMOUNT, axc_ctx_p, &axcbundle_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to collect axc bundle");
        goto cleanup;
    }

    ret_val = omemo_bundle_create(&omemobundle_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to create omemo_bundle");
        goto cleanup;
    }

    ret_val = omemo_bundle_set_device_id(omemobundle_p, axc_bundle_get_reg_id(axcbundle_p));
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to set device id in omemo bundle");
        goto cleanup;
    }

    curr_buf_p = axc_bundle_get_signed_pre_key(axcbundle_p);
    ret_val = omemo_bundle_set_signed_pre_key(omemobundle_p,
                                              axc_bundle_get_signed_pre_key_id(axcbundle_p),
                                              axc_buf_get_data(curr_buf_p),
                                              axc_buf_get_len(curr_buf_p));
    if(ret_val) {
        err_msg_dbg = g_strdup_printf("failed to set signed pre key in omemo bundle");
        goto cleanup;
    }

    curr_buf_p = axc_bundle_get_signature(axcbundle_p);
    ret_val = omemo_bundle_set_signature(omemobundle_p,
                                         axc_buf_get_data(curr_buf_p),
                                         axc_buf_get_len(curr_buf_p));
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to set signature in omemo bundle");
        goto cleanup;
    }

    curr_buf_p = axc_bundle_get_identity_key(axcbundle_p);
    ret_val = omemo_bundle_set_identity_key(omemobundle_p,
                                            axc_buf_get_data(curr_buf_p),
                                            axc_buf_get_len(curr_buf_p));
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to set public identity key in omemo bundle");
        goto cleanup;
    }

    next_p = axc_bundle_get_pre_key_list(axcbundle_p);
    while (next_p) {
        curr_buf_p = axc_buf_list_item_get_buf(next_p);
        ret_val = omemo_bundle_add_pre_key(omemobundle_p,
                                           axc_buf_list_item_get_id(next_p),
                                           axc_buf_get_data(curr_buf_p),
                                           axc_buf_get_len(curr_buf_p));
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to add public pre key to omemo bundle");
            goto cleanup;
        }
        next_p = axc_buf_list_item_get_next(next_p);
    }

    ret_val = omemo_bundle_export(omemobundle_p, &bundle_xml);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to export omemo bundle to xml");
        goto cleanup;
    }

    //publish_node_bundle_p = xmlnode_from_str(bundle_xml, -1);
    //jabber_pep_publish(js_p, publish_node_bundle_p);
    //std::cout << "publish pep (bundle): " << bundle_xml;

    publishPepRawXml(client_->getJID(), QString(bundle_xml));

    /*
     * <publish node="eu.siacs.conversations.axolotl.bundles:1394567069">
     *   <item>
     *     <bundle xmlns="eu.siacs.conversations.axolotl">
     *       <signedPreKeyPublic signedPreKeyId="0">
     *         BVR1y9qOhyZXjDyG3agwNVZ9cAAa0EkkvtHIAec5ggBB
     *       </signedPreKeyPublic>
     *       <signedPreKeySignature>
     *         j//Z4tVTpC1xCb4guO+ADio7aaR3Vay2jL2FVZnry6yA2Rn8o9R6yyOq8W7/cKqiJMXDqyNS2AdKJ2Aho2uSjA==
     *       </signedPreKeySignature>
     *       <identityKey>
     *         BUFpuM6vO4PXetzYWojojqJ/zTmmtJXm8CGery99gKIO
     *       </identityKey>
     *       <prekeys>
     *         <preKeyPublic preKeyId="1">
     *           BZsyrh4JjrtfqI8wNQNtLtZeKD05nS72lEDKc03Hwbxw
     *         </preKeyPublic>
     *         <preKeyPublic preKeyId="2">
     *           Bb6l5Eyw+syqnqtfCd/bqtMOdqgLshhagV5McHDHbusB
     *         </preKeyPublic>
     *         <preKeyPublic preKeyId="3">
     *           BZUEqj4SznQw5RUsRYqBiGXdJjqtHymHu9eMyIV9bcwF
     *         </preKeyPublic>
     *         ...
     */

    purple_debug_info("lurch", "%s: published own bundle for %s\n", __func__, uname);

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }

    axc_context_destroy_all(axc_ctx_p);
    axc_bundle_destroy(axcbundle_p);
    omemo_bundle_destroy(omemobundle_p);
    free(bundle_xml);

    return ret_val;
}

/**
 * Processes a devicelist by updating the database with it.
 *
 * @param uname The username.
 * @param dl_in_p Pointer to the incoming devicelist.
 * @param js_p Pointer to the JabberStream.
 * @return 0 on success, negative on error.
 */
int Omemo::lurch_devicelist_process(char * uname, omemo_devicelist * dl_in_p /*, JabberStream * js_p*/) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    const char * from = nullptr;
    char * db_fn_omemo = nullptr;
    axc_context * axc_ctx_p = nullptr;
    omemo_devicelist * dl_db_p = nullptr;
    GList * add_l_p = nullptr;
    GList * del_l_p = nullptr;
    GList * curr_p = nullptr;
    uint32_t curr_id = 0;
    char * bundle_node_name = nullptr;

    char * temp;

    from = omemo_devicelist_get_owner(dl_in_p);
    db_fn_omemo = lurch_uname_get_db_fn(uname, LURCH_DB_NAME_OMEMO);

    purple_debug_info("lurch", "%s: processing devicelist from %s for %s\n", __func__, from, uname);

    ret_val = omemo_storage_user_devicelist_retrieve(from, db_fn_omemo, &dl_db_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to retrieve cached devicelist for %s from db %s", from, db_fn_omemo);
        goto cleanup;
    }

    omemo_devicelist_export(dl_db_p, &temp);
    purple_debug_info("lurch", "%s: %s\n%s\n", __func__, "cached devicelist is", temp);

    ret_val = omemo_devicelist_diff(dl_in_p, dl_db_p, &add_l_p, &del_l_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to diff devicelists");
        goto cleanup;
    }

    for (curr_p = add_l_p; curr_p; curr_p = curr_p->next) {
        curr_id = omemo_devicelist_list_data(curr_p);
        purple_debug_info("lurch", "%s: saving %i for %s to db %s\n", __func__, curr_id, from, db_fn_omemo);
        ret_val = omemo_storage_user_device_id_save(from, curr_id, db_fn_omemo);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to save %i for %s to %s", curr_id, from, db_fn_omemo);
            goto cleanup;
        }
    }

    ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to init axc ctx");
        goto cleanup;
    }

    for (curr_p = del_l_p; curr_p; curr_p = curr_p->next) {
        curr_id = omemo_devicelist_list_data(curr_p);
        purple_debug_info("lurch", "%s: deleting %i for %s to db %s\n", __func__, curr_id, from, db_fn_omemo);

        ret_val = omemo_storage_user_device_id_delete(from, curr_id, db_fn_omemo);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to delete %i for %s from %s", curr_id, from, db_fn_omemo);
            goto cleanup;
        }
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }
    free(db_fn_omemo);
    omemo_devicelist_destroy(dl_db_p);
    axc_context_destroy_all(axc_ctx_p);
    g_list_free_full(add_l_p, free);
    g_list_free_full(del_l_p, free);
    free(bundle_node_name);
    free(temp);

    return ret_val;
}

/*
 * Strips the <pubsub ..> root element and returns the xml starting with next node, which must be a
 *              <itmes ..> tag
 */
QString Omemo::getItemsChildFromPubSubXml(const QString &xml)
{
    QString returnXml = "";

    QDomDocument d;
    d.setContent(xml);

    QDomElement element = d.documentElement();
    QDomNode n = element.firstChildElement("items");
    if (! n.isNull())
    {
        QDomDocument dd("");
        dd.appendChild(n);
        returnXml = dd.toString();
    }

    return returnXml;
}

/*
 * Mock mock.
 * Catch purple debug messages
 */
void Omemo::purple_debug_info(const char *category, const char *format,...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void Omemo::purple_debug_error(const char *category, const char *format,...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
