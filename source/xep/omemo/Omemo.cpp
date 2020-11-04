#include "Omemo.h"
#include "System.h"
#include "XmlProcessor.h"
#include "Settings.h"
#include "System.h"

#include <QDir>
#include <QDomDocument>
#include <QDebug>

extern "C" {
#include "libomemo_crypto.h"
#include "libomemo_storage.h"
#include "axc_store.h"
}

// see https://www.ietf.org/rfc/rfc3920.txt
#define JABBER_MAX_LEN_NODE 1023
#define JABBER_MAX_LEN_DOMAIN 1023
#define JABBER_MAX_LEN_BARE JABBER_MAX_LEN_NODE + JABBER_MAX_LEN_DOMAIN + 1

#define LURCH_DB_SUFFIX "_db.sqlite"
#define LURCH_DB_NAME_OMEMO "omemo"
#define LURCH_DB_NAME_AXC "axc"

#define LURCH_ERR           -1000000
#define LURCH_ERR_NOMEM     -1000001
#define LURCH_ERR_NO_BUNDLE -1000010

#define LURCH_ERR_STRING_ENCRYPT "There was an error encrypting the message and it was not sent. " \
                                 "You can try again, or try to find the problem by looking at the debug log."
#define LURCH_ERR_STRING_DECRYPT "There was an error decrypting an OMEMO message addressed to this device. " \
                                 "See the debug log for details."

/*
 * https://xmpp.org/extensions/xep-0384.html
 * https://conversations.im/omemo/xep-omemo.html
 *
 * 1. check if own id is in my device list. update if necessary. give read access to world
 * 2. check all contacts if they support omemo. must cache that list
 * 3. create and publish bundle. give read access to world
 * 4. build a session (fetch other clients bundle information)
 * 5. encrypt and send the message
 * 6. receive and decrypt a message
 */

/*
 * This file is ported from lurch
 * https://github.com/gkdr/lurch
 *
 * Thank you for all the great work!
 */

void purple_debug_info (const char *category, const char *format,...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void purple_debug_error(const char *category, const char *format,...)
{
    purple_debug_info(category, format);
}

Omemo::Omemo(QObject *parent) : QObject(parent)
{
    // lurch_plugin_load

    // create omemo path if needed
    QString omemoLocation = System::getOmemoPath();
    QDir dir(omemoLocation);

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // init omemo crypto
    omemo_default_crypto_init();

    // get the deviceListNodeName and save it for later use
    char* dlNs = nullptr;
    int ret_val = omemo_devicelist_get_pep_node_name(&dlNs);
    if (ret_val != 0)
    {
        qDebug() << "failed to get devicelist pep node name";
    }
    else
    {
        deviceListNodeName_ = QString::fromUtf8(dlNs);
        free(dlNs);
    }

    qDebug() << "Omemo::Omemo" << deviceListNodeName_;
}

Omemo::~Omemo()
{
    omemo_default_crypto_teardown();
    free(uname_);
}

void Omemo::setupWithClient(Swift::Client* client)
{
    client_ = client;

    myBareJid_ = QString::fromStdString(client_->getJID().toBare().toString());
    uname_ = strdup(myBareJid_.toStdString().c_str());

    client_->onDataRead.connect(boost::bind(&Omemo::handleDataReceived, this, _1));
    client_->onConnected.connect(boost::bind(&Omemo::handleConnected, this));

    requestDeviceList(client_->getJID());
}

void Omemo::requestDeviceList(const Swift::JID& jid)
{
    /*request
     * <iq id="87f4e888-fb43-4e4d-a1f8-09884b623f71" to="xxx@jabber-germany.de" type="get">
         <pubsub xmlns="http://jabber.org/protocol/pubsub">
          <items node="eu.siacs.conversations.axolotl.devicelist"></items>
         </pubsub>
        </iq>
     */

    /*answer
     * <iq to="xxx@jabber.ccc.de/shmoose" from="xxx@jabber-germany.de" type="result" id="87f4e888-fb43-4e4d-a1f8-09884b623f71">
         <pubsub xmlns="http://jabber.org/protocol/pubsub">
          <items node="eu.siacs.conversations.axolotl.devicelist">
               <item id="6445fd7d-6d65-4a2e-829b-9e2f6f0f0d53">
                <list xmlns="eu.siacs.conversations.axolotl">
                 <device id="1234567"></device>
                </list>
               </item>
          </items>
         </pubsub>
        </iq>
     */


    // gen the payload
    const std::string deviceListRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + deviceListNodeName_.toStdString() + "'/></pubsub>";

    Swift::RawRequest::ref requestDeviceList = Swift::RawRequest::create(Swift::IQ::Get, jid.toBare(), deviceListRequestXml, client_->getIQRouter());
    requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1));
    requestDeviceList->send();

    std::string id{requestDeviceList->getID()};
    requestedDeviceListJidIdMap_.insert(QString::fromStdString(id), QString::fromStdString(jid.toBare().toString()));
}

void Omemo::handleDeviceListResponse(const std::string& str)
{
    // implements lurch_pep_devicelist_event_handler

    qDebug() << "OMEMO: handle device list Response: " << QString::fromStdString(str);

    /*
     *  <pubsub xmlns="http://jabber.org/protocol/pubsub">
  <items node="eu.siacs.conversations.axolotl.devicelist">
   <item id="5ED52F653A338">
    <list xmlns="eu.siacs.conversations.axolotl">
     <device id="24234234"></device>
    </list>
   </item>
  </items>
 </pubsub>

    OR

    <error type=\"cancel\"><item-not-found xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/></error>
     */

    /*
     * see lurch_pep_devicelist_event_handler
     * 1. check if this is our device list?!
     * -> lurch_pep_own_devicelist_request_handler
     * 2. else
     * -> omemo_devicelist_import
     * -> lurch_devicelist_process
     */


    // determine for whom this device list was requested for
    QString currentIqId = XmlProcessor::getContentInTag("iq", "id", currentNode_);

    if ( (! currentIqId.isEmpty()) && requestedDeviceListJidIdMap_.contains(currentIqId))
    {
        QString jid = requestedDeviceListJidIdMap_.value(currentIqId);
        requestedDeviceListJidIdMap_.remove(currentIqId);

        //qDebug() << "jid " << jid;
        // get the items of the request
        QString items = XmlProcessor::getChildFromNode("items", currentNode_);

        if (! items.isEmpty())
        {
            if (myBareJid_.compare(jid, Qt::CaseInsensitive) == 0)
            {
                // was a request for my device list
                ownDeviceListRequestHandler(myBareJid_, items);
            }
            else
            {
                // requested someone else device list
                qDebug() << "requested someone else device list";

                omemo_devicelist* dl_in_p = nullptr;
                char* pItems = strdup(items.toStdString().c_str());

                if (omemo_devicelist_import(pItems, jid.toStdString().c_str(), &dl_in_p) != 0)
                {
                    if(devicelistProcess(jid.toStdString().c_str(), dl_in_p) != 0)
                    {
                        qDebug() << "failed to process devicelist";
                    }

                    omemo_devicelist_destroy(dl_in_p);
                }
                else
                {
                   qDebug() << "failed to import devicelist";
                }
                free(pItems);
            }
        }
    }
}

void Omemo::ownDeviceListRequestHandler(QString fromJid, QString items)
{
    // implements 4.3 Announcing support -> device list

    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    int len = 0;
    int install = 0;
    axc_context * axc_ctx_p = nullptr;
    uint32_t own_id = 0;
    int needs_publishing = 1;
    omemo_devicelist * dl_p = nullptr;
    char * dl_xml = nullptr;

    Swift::RawRequest::ref publishPep{};
    std::string devicelist{};

    //int uninstall_ = 0; // FIXME implement me!

    Settings settings;
    install = (settings.isOmemoInitialized() == false) ? 1 : 0;

    if (install && !uninstall_) {
      purple_debug_info("lurch", "%s: %s\n", __func__, "preparing installation...");
      ret_val = axcPrepare(uname_);
      if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to prepare axc");
        goto cleanup;
      }
      purple_debug_info("lurch", "%s: %s\n", __func__, "...done");
    }

    ret_val = axcGetInitCtx(&axc_ctx_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to init axc ctx");
      goto cleanup;
    }
    ret_val = axc_get_device_id(axc_ctx_p, &own_id);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to get own id");
      goto cleanup;
    }

    if (items.isEmpty() == true) {
      purple_debug_info("lurch", "%s: %s\n", __func__, "no devicelist yet, creating it");
      ret_val = omemo_devicelist_create(uname_, &dl_p);
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
      char* pItems = strdup(items.toStdString().c_str());
      ret_val = omemo_devicelist_import(pItems, uname_, &dl_p);
      free(pItems);
      if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to import received devicelist");
        goto cleanup;
      }

      ret_val = omemo_devicelist_contains_id(dl_p, own_id);
      if (ret_val == 1) {
        purple_debug_info("lurch", "%s: %s\n", __func__, "own id was already contained in received devicelist, doing nothing");
        needs_publishing = 0;
      } else if (ret_val == 0) {
        if (!uninstall_) {
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

      devicelist = "<pubsub xmlns='http://jabber.org/protocol/pubsub'>" + std::string(dl_xml) + "</pubsub>";
      publishPep = Swift::RawRequest::create(Swift::IQ::Set, uname_, devicelist, client_->getIQRouter());
      publishPep->onResponse.connect(boost::bind(&Omemo::publishedDeviceList, this, _1));
      publishPep->send();

      purple_debug_info("lurch", "%s: \n%s:\n", __func__, "...done");
    }

    ret_val = bundlePublishOwn();
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to publish own bundle");
      goto cleanup;
    }

    if (install && !uninstall_) {
      //purple_account_set_bool(acc_p, LURCH_ACC_SETTING_INITIALIZED, TRUE);
      settings.setOmemoInitialized(true);
    }

    ret_val = devicelistProcess(uname_, dl_p);
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

void Omemo::publishedDeviceList(const std::string& str)
{
    // FIXME check if there was an error on device list publishing
    qDebug() << "OMEMO: publishedDeviceList: " << QString::fromStdString(str);
}

int Omemo::devicelistProcess(const char* uname, omemo_devicelist * dl_in_p)
{
    // implements lurch_devicelist_process
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

    char * debug_str = nullptr;

    from = omemo_devicelist_get_owner(dl_in_p);
    db_fn_omemo = unameGetDbFn(uname, (char*)LURCH_DB_NAME_OMEMO);

    purple_debug_info("lurch", "%s: processing devicelist from %s for %s\n", __func__, from, uname);

    ret_val = omemo_storage_user_devicelist_retrieve(from, db_fn_omemo, &dl_db_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to retrieve cached devicelist for %s from db %s", from, db_fn_omemo);
      goto cleanup;
    }

    omemo_devicelist_export(dl_db_p, &debug_str);
    purple_debug_info("lurch", "%s: %s\n%s\n", __func__, "cached devicelist is", debug_str);

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

    ret_val = axcGetInitCtx(&axc_ctx_p);
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
    free(debug_str);

    return ret_val;

}

int Omemo::bundlePublishOwn()
{
    // implements lurch_bundle_publish_own
    // // implements 4.3 Announcing support -> bundle

    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    axc_context * axc_ctx_p = nullptr;
    axc_bundle * axcbundle_p = nullptr;
    omemo_bundle * omemobundle_p = nullptr;
    axc_buf * curr_buf_p = nullptr;
    axc_buf_list_item * next_p = nullptr;
    char * bundle_xml = nullptr;

    Swift::RawRequest::ref publishPep{};
    std::string bundle{};

    ret_val = axcGetInitCtx(&axc_ctx_p);
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

    bundle = "<pubsub xmlns='http://jabber.org/protocol/pubsub'>" + std::string(bundle_xml) + "</pubsub>";

    publishPep = Swift::RawRequest::create(Swift::IQ::Set, uname_, bundle, client_->getIQRouter());
    publishPep->onResponse.connect(boost::bind(&Omemo::publishedBundle, this, _1));
    publishPep->send();

    purple_debug_info("lurch", "%s: published own bundle for %s\n", __func__, uname_);

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

void Omemo::publishedBundle(const std::string& str)
{
    // FIXME check if there was an error on bundle publishing
    qDebug() << "OMEMO: publishedBundle: " << QString::fromStdString(str);
}


bool Omemo::axcPrepare(QString fromJid)
{
    // implements lurch_axc_prepare
    int ret_val = 0;

    char * err_msg_dbg = nullptr;
    axc_context * axc_ctx_p = nullptr;
    uint32_t device_id = 0;
    char * db_fn_omemo = nullptr;

    ret_val = axcGetInitCtx(&axc_ctx_p);
    if (ret_val)
    {
        err_msg_dbg = g_strdup_printf("failed to get init axc ctx");
        goto cleanup;
    }

    ret_val = axc_get_device_id(axc_ctx_p, &device_id);
    if (!ret_val)
    {
        // already installed
        goto cleanup;
    }

    db_fn_omemo = unameGetDbFn(fromJid.toStdString().c_str(), (char *)LURCH_DB_NAME_OMEMO);

    while (1)
    {
        ret_val = axc_install(axc_ctx_p);
        if (ret_val)
        {
            err_msg_dbg = g_strdup_printf("failed to install axc");
            goto cleanup;
        }

        ret_val = axc_get_device_id(axc_ctx_p, &device_id);
        if (ret_val)
        {
            err_msg_dbg = g_strdup_printf("failed to get own device id");
            goto cleanup;
        }

        ret_val = omemo_storage_global_device_id_exists(device_id, db_fn_omemo);
        if (ret_val == 1)
        {
            ret_val = axc_db_init_status_set(AXC_DB_NEEDS_ROLLBACK, axc_ctx_p);
            if (ret_val)
            {
                err_msg_dbg = g_strdup_printf("failed to set axc db to rollback");
                goto cleanup;
            }
        } else if (ret_val < 0)
        {
            err_msg_dbg = g_strdup_printf("failed to access the db %s", db_fn_omemo);
            goto cleanup;
        }
        else
        {
            break;
        }
    }

cleanup:
    if (err_msg_dbg)
    {
        //purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        qDebug() << "omemo: " << __func__ << ", " << err_msg_dbg << ", " << ret_val;
        free(err_msg_dbg);
    }
    axc_context_destroy_all(axc_ctx_p);
    free(db_fn_omemo);

    return ret_val;

}

bool Omemo::axcGetInitCtx(axc_context** ctx_pp)
{
    // implements lurch_axc_get_init_ctx
    int ret_val = 0;
    char* err_msg_dbg = nullptr;

    axc_context* ctx_p = nullptr;
    char* db_fn = nullptr;

    ret_val = axc_context_create(&ctx_p);
    if (ret_val)
    {
        err_msg_dbg = g_strdup_printf("failed to create axc context");
        goto cleanup;
    }

    db_fn = unameGetDbFn(myBareJid_.toStdString().c_str(), (char*)LURCH_DB_NAME_AXC);
    ret_val = axc_context_set_db_fn(ctx_p, db_fn, strlen(db_fn));
    if (ret_val)
    {
        err_msg_dbg = g_strdup_printf("failed to set axc db filename");
        goto cleanup;
    }

    ret_val = axc_init(ctx_p);
    if (ret_val)
    {
        err_msg_dbg = g_strdup_printf("failed to init axc context");
        goto cleanup;
    }

    *ctx_pp = ctx_p;

cleanup:
    if (ret_val)
    {
        axc_context_destroy_all(ctx_p);
    }
    if (err_msg_dbg)
    {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }

    free (db_fn);
    return ret_val;
}

void Omemo::accountConnectCb()
{
    // lurch_account_connect_cb
    // functionality already reflected in setupWithClient which is called after login

}

void Omemo::handleConnected()
{
    accountConnectCb();
}

char* Omemo::unameGetDbFn(const char * uname, char * which)
{
    // impelements lurch_uname_get_db_fn
    return g_strconcat(System::getOmemoPath().toStdString().c_str(), "/", uname, "_", which, LURCH_DB_SUFFIX, NULL);
}

void Omemo::handleDataReceived(Swift::SafeByteArray data)
{
    // jabber xml stream from swift to xmlnode
    std::string nodeData = Swift::safeByteArrayToString(data);
    currentNode_ = QString::fromStdString(nodeData);

    if (isEncryptedMessage(QString::fromStdString(nodeData)) == true)
    {
        messageDecrypt(nodeData);
    }

}

void Omemo::messageDecrypt(const std::string& message)
{
    // lurch_message_decrypt
    int ret_val = 0;
    char * err_msg_dbg = nullptr;
    int len;

    omemo_message * msg_p = nullptr;
    char * uname = nullptr;
    char * db_fn_omemo = nullptr;
    axc_context * axc_ctx_p = nullptr;
    uint32_t own_id = 0;
    uint8_t * key_p = nullptr;
    size_t key_len = 0;
    axc_buf * key_buf_p = nullptr;
    axc_buf * key_decrypted_p = nullptr;
    char * sender_name = nullptr;
    axc_address sender_addr{};
    char * bundle_node_name = nullptr;
    omemo_message * keytransport_msg_p = nullptr;
    char * xml = nullptr;
    char * sender = nullptr;
    char ** split = nullptr;
    char * room_name = nullptr;
    char * buddy_nick = nullptr;
    //xmlnode * plaintext_msg_node_p = nullptr;
    char * recipient_bare_jid = nullptr;
    char* pMsg{nullptr};

    //const char * type = xmlnode_get_attrib(*msg_stanza_pp, "type");
    //const char * from = xmlnode_get_attrib(*msg_stanza_pp, "from");
    const char* type{nullptr};
    const char* from{nullptr};
    QString qsType = XmlProcessor::getContentInTag("message", "type", QString::fromStdString(message));
    QString qsFrom = XmlProcessor::getContentInTag("message", "from", QString::fromStdString(message));
    if ( (!qsType.isEmpty()) && (!qsFrom.isEmpty()))
    {
        type = qsType.toStdString().c_str();
        from = qsFrom.toStdString().c_str();
    }
    else
    {
        qDebug() << "from or type not found in message node!";
    }

    if (uninstall_) {
      goto cleanup;
    }

    //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(gc_p)));
    db_fn_omemo = unameGetDbFn(uname_, (char*)LURCH_DB_NAME_OMEMO);

    if (!g_strcmp0(type, "chat")) {
      //sender = jabber_get_bare_jid(from);
      sender = strdup(Swift::JID(from).toBare().toString().c_str());

      ret_val = omemo_storage_chatlist_exists(sender, db_fn_omemo);
      if (ret_val < 0) {
        err_msg_dbg = g_strdup_printf("failed to look up %s in %s", sender, db_fn_omemo);
        goto cleanup;
      } else if (ret_val == 1) {
        //purple_conv_present_error(sender, purple_connection_get_account(gc_p), "Received encrypted message in blacklisted conversation.");
          qDebug() << "Received encrypted message in blacklisted conversation."; // FIXME show to user
      }
    } else if (!g_strcmp0(type, "groupchat")) {
      split = g_strsplit(from, "/", 2);
      room_name = split[0];
      buddy_nick = split[1];

      ret_val = omemo_storage_chatlist_exists(room_name, db_fn_omemo);
      if (ret_val < 0) {
        err_msg_dbg = g_strdup_printf("failed to look up %s in %s", room_name, db_fn_omemo);
        goto cleanup;
      } else if (ret_val == 0) {
        //purple_conv_present_error(room_name, purple_connection_get_account(gc_p), "Received encrypted message in non-OMEMO room.");
          qDebug() << "Received encrypted message in non-OMEMO room."; // FIXME show to user
      }
#if 0
      // FIXME implement the group chat decryption stuff
      conv_p = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, room_name, purple_connection_get_account(gc_p));
      if (!conv_p) {
        err_msg_dbg = g_strdup_printf("could not find groupchat %s", room_name);
        goto cleanup;
      }

      muc_p = jabber_chat_find_by_conv(conv_p);
      if (!muc_p) {
        err_msg_dbg = g_strdup_printf("could not find muc struct for groupchat %s", room_name);
        goto cleanup;
      }

      muc_member_p = g_hash_table_lookup(muc_p->members, buddy_nick);
      if (!muc_member_p) {
        purple_debug_misc("lurch", "Received OMEMO message in MUC %s, but the sender %s is not present in the room, which can happen during history catchup. Skipping.\n", room_name, buddy_nick);
        goto cleanup;
      }

      if (!muc_member_p->jid) {
        err_msg_dbg = g_strdup_printf("jid for user %s in muc %s not found, is the room anonymous?", buddy_nick, room_name);
        goto cleanup;
      }

      sender = jabber_get_bare_jid(muc_member_p->jid);
#endif
    }

    pMsg = strdup(message.c_str());
    ret_val = omemo_message_prepare_decryption(pMsg, &msg_p);
    free(pMsg);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed import msg for decryption");
      goto cleanup;
    }

    ret_val = axcGetInitCtx(&axc_ctx_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to get axc ctx for %s", uname);
      goto cleanup;
    }

    ret_val = axc_get_device_id(axc_ctx_p, &own_id);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to get own device id");
      goto cleanup;
    }

    ret_val = omemo_message_get_encrypted_key(msg_p, own_id, &key_p, &key_len);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to get key for own id %i", own_id);
      goto cleanup;
    }
    if (!key_p) {
      purple_debug_info("lurch", "received omemo message that does not contain a key for this device, skipping\n");
      goto cleanup;
    }

    key_buf_p = axc_buf_create(key_p, key_len);
    if (!key_buf_p) {
      err_msg_dbg = g_strdup_printf("failed to create buf for key");
      goto cleanup;
    }

    sender_addr.name = sender;
    sender_addr.name_len = strnlen(sender_addr.name, JABBER_MAX_LEN_BARE);
    sender_addr.device_id = omemo_message_get_sender_id(msg_p);

    ret_val = axc_pre_key_message_process(key_buf_p, &sender_addr, axc_ctx_p, &key_decrypted_p);
    if (ret_val == AXC_ERR_NOT_A_PREKEY_MSG) {
      if (axc_session_exists_initiated(&sender_addr, axc_ctx_p)) {
        ret_val = axc_message_decrypt_from_serialized(key_buf_p, &sender_addr, axc_ctx_p, &key_decrypted_p);
        if (ret_val) {
          err_msg_dbg = g_strdup_printf("failed to decrypt key");
          goto cleanup;
        }
      } else {
        purple_debug_info("lurch", "received omemo message but no session with the device exists, ignoring\n");
        goto cleanup;
      }
    } else if (ret_val == AXC_ERR_INVALID_KEY_ID) {
      ret_val = omemo_bundle_get_pep_node_name(sender_addr.device_id, &bundle_node_name);
      if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get bundle pep node name");
        goto cleanup;
      }

#if 0
      jabber_pep_request_item(purple_connection_get_protocol_data(gc_p),
                              sender_addr.name, bundle_node_name,
                              (void *) 0,
                              lurch_pep_bundle_for_keytransport);
#endif
      qDebug() << "FIXME implement pep request for " << sender_addr.name << ", " << bundle_node_name << " to call lurch_pep_bundle_for_keytransport";

    } else if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to prekey msg");
      goto cleanup;
    } else {
      //lurch_bundle_publish_own(purple_connection_get_protocol_data(gc_p));
        bundlePublishOwn();
    }

    if (!omemo_message_has_payload(msg_p)) {
      purple_debug_info("lurch", "received keytransportmsg\n");
      goto cleanup;
    }

    ret_val = omemo_message_export_decrypted(msg_p, axc_buf_get_data(key_decrypted_p), axc_buf_get_len(key_decrypted_p), &crypto, &xml);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to decrypt payload");
      goto cleanup;
    }


    qDebug() << "decrypted msg: " << xml;
#if 0
    plaintext_msg_node_p = xmlnode_from_str(xml, -1);

    // libpurple doesn't know what to do with incoming messages addressed to someone else, so they need to be written to the conversation manually
    // incoming messages from the own account in MUCs are fine though
    if (!g_strcmp0(sender, uname) && !g_strcmp0(type, "chat")) {
      recipient_bare_jid = jabber_get_bare_jid(xmlnode_get_attrib(*msg_stanza_pp, "to"));
      conv_p = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, sender, purple_connection_get_account(gc_p));
      if (!conv_p) {
        conv_p = purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_connection_get_account(gc_p), recipient_bare_jid);
      }
      purple_conversation_write(conv_p, uname, xmlnode_get_data(xmlnode_get_child(plaintext_msg_node_p, "body")), PURPLE_MESSAGE_SEND, time((void *) 0));
      *msg_stanza_pp = (void *) 0;
    } else {
      *msg_stanza_pp = plaintext_msg_node_p;
    }
#endif

  cleanup:
    if (err_msg_dbg) {
      //purple_conv_present_error(sender, purple_connection_get_account(gc_p), LURCH_ERR_STRING_DECRYPT); FIXME show to user
      purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
      free(err_msg_dbg);
    }

    g_strfreev(split);
    free(sender);
    free(xml);
    free(bundle_node_name);
    free(sender_name);
    axc_buf_free(key_decrypted_p);
    axc_buf_free(key_buf_p);
    free(key_p);
    axc_context_destroy_all(axc_ctx_p);
    free(uname);
    free(db_fn_omemo);
    free(recipient_bare_jid);
    omemo_message_destroy(keytransport_msg_p);
    omemo_message_destroy(msg_p);
}

void Omemo::pepBundleForKeytransport(const std::string from, const std::string &items)
{
    // lurch_pep_bundle_for_keytransport
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    //char * uname = (void *) 0;
    axc_context * axc_ctx_p{nullptr};
    uint32_t own_id{0};
    omemo_message * msg_p{nullptr};
    axc_address addr{};
    lurch_addr laddr{};
    axc_buf * key_ct_buf_p;
    char * msg_xml{nullptr};
    //xmlnode * msg_node_p{nullptr};
    //void * jabber_handle_p = purple_plugins_find_with_id("prpl-jabber");

    //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(js_p->gc)));

    // FIXME check the struct if information are parsed correct
    addr.name = from.c_str();
    //addr.name_len = strnlen(from, JABBER_MAX_LEN_BARE);
    addr.name_len = from.size();
    //addr.device_id = lurch_bundle_name_get_device_id(xmlnode_get_attrib(items_p, "node"));
    addr.device_id = atoi(XmlProcessor::getContentInTag("item", "id", QString::fromStdString(items)).toStdString().c_str());

    purple_debug_info("lurch", "%s: %s received bundle from %s:%i\n", __func__, uname_, from.c_str(), addr.device_id);

    laddr.jid = g_strndup(addr.name, addr.name_len);
    laddr.device_id = addr.device_id;

    ret_val = axcGetInitCtx(&axc_ctx_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to init axc ctx");
      goto cleanup;
    }

    // make sure it's gonna be a pre_key_message
    ret_val = axc_session_delete(addr.name, addr.device_id, axc_ctx_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to delete possibly existing session");
      goto cleanup;
    }

    ret_val = bundleCreateSession(from.c_str(), items, axc_ctx_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to create session");
      goto cleanup;
    }

    purple_debug_info("lurch", "%s: %s created session with %s:%i\n", __func__, uname_, from.c_str(), addr.device_id);

    ret_val = axc_get_device_id(axc_ctx_p, &own_id);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to get own device id");
      goto cleanup;
    }

    ret_val = omemo_message_create(own_id, &crypto, &msg_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to create omemo msg");
      goto cleanup;
    }

    ret_val = keyEncrypt(&laddr,
                                omemo_message_get_key(msg_p),
                                omemo_message_get_key_len(msg_p),
                                axc_ctx_p,
                                &key_ct_buf_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to encrypt key for %s:%i", addr.name, addr.device_id);
      goto cleanup;
    }

    ret_val = omemo_message_add_recipient(msg_p,
                                          addr.device_id,
                                          axc_buf_get_data(key_ct_buf_p),
                                          axc_buf_get_len(key_ct_buf_p));
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to add %s:%i as recipient to message", addr.name, addr.device_id);
      goto cleanup;
    }

    // don't call wrapper function here as EME is not necessary
    ret_val = omemo_message_export_encrypted(msg_p, OMEMO_ADD_MSG_NONE, &msg_xml);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("failed to export encrypted msg");
      goto cleanup;
    }

#if 0
    msg_node_p = xmlnode_from_str(msg_xml, -1);
    if (!msg_node_p) {
      err_msg_dbg = g_strdup_printf("failed to create xml node from xml string");
      goto cleanup;
    }
#endif
    qDebug() << "FIXME keytransport msg: " << msg_xml;
    //purple_signal_emit(jabber_handle_p, "jabber-sending-xmlnode", js_p->gc, &msg_node_p);

    purple_debug_info("lurch", "%s: %s sent keytransportmsg to %s:%i\n", __func__, uname_, from.c_str(), addr.device_id);


  cleanup:
    if (err_msg_dbg) {
      purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
      free(err_msg_dbg);
    }
    free(laddr.jid);
    axc_context_destroy_all(axc_ctx_p);
    omemo_message_destroy(msg_p);
    axc_buf_free(key_ct_buf_p);
    free(msg_xml);
}


int Omemo::keyEncrypt(const lurch_addr * recipient_addr_p, const uint8_t * key_p, size_t key_len, axc_context * axc_ctx_p, axc_buf ** key_ct_buf_pp)
{
    // lurch_key_encrypt
    int ret_val{0};
    char * err_msg_dbg{nullptr};

    axc_buf * key_buf_p{nullptr};
    axc_buf * key_ct_buf_p{nullptr};
    axc_address axc_addr{};

    purple_debug_info("lurch", "%s: encrypting key for %s:%i\n", __func__, recipient_addr_p->jid, recipient_addr_p->device_id);

    key_buf_p = axc_buf_create(key_p, key_len);
    if (!key_buf_p) {
        err_msg_dbg = g_strdup_printf("failed to create buffer for the key");
        goto cleanup;
    }

    axc_addr.name = recipient_addr_p->jid;
    axc_addr.name_len = strnlen(axc_addr.name, JABBER_MAX_LEN_BARE);
    axc_addr.device_id = recipient_addr_p->device_id;

    ret_val = axc_message_encrypt_and_serialize(key_buf_p, &axc_addr, axc_ctx_p, &key_ct_buf_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to encrypt the key");
        goto cleanup;
    }

    *key_ct_buf_pp = key_ct_buf_p;

cleanup:
    if (ret_val) {
        axc_buf_free(key_ct_buf_p);
    }
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }
    axc_buf_free(key_buf_p);

    return ret_val;
}


int Omemo::bundleCreateSession(const char* from, const std::string& items, axc_context * axc_ctx_p)
{
    // lurch_bundle_create_session
    int ret_val{0};
    char * err_msg_dbg{nullptr};

    int len{};
    omemo_bundle * om_bundle_p{nullptr};
    axc_address remote_addr{};
    uint32_t pre_key_id{0};
    uint8_t * pre_key_p{nullptr};
    size_t pre_key_len{0};
    uint32_t signed_pre_key_id{0};
    uint8_t * signed_pre_key_p{nullptr};
    size_t signed_pre_key_len{0};
    uint8_t * signature_p{nullptr};
    size_t signature_len{0};
    uint8_t * identity_key_p{nullptr};
    size_t identity_key_len{0};
    axc_buf * pre_key_buf_p{nullptr};
    axc_buf * signed_pre_key_buf_p{nullptr};
    axc_buf * signature_buf_p{nullptr};
    axc_buf * identity_key_buf_p{nullptr};

    purple_debug_info("lurch", "%s: creating a session between %s and %s from a received bundle\n", __func__, uname_, from);

    ret_val = omemo_bundle_import(items.c_str(), &om_bundle_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to import xml into bundle");
        goto cleanup;
    }

    remote_addr.name = from;
    remote_addr.name_len = strnlen(from, JABBER_MAX_LEN_BARE);
    remote_addr.device_id = omemo_bundle_get_device_id(om_bundle_p);

    purple_debug_info("lurch", "%s: bundle's device id is %i\n", __func__, remote_addr.device_id);

    ret_val = omemo_bundle_get_random_pre_key(om_bundle_p, &pre_key_id, &pre_key_p, &pre_key_len);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed get a random pre key from the bundle");
        goto cleanup;
    }
    ret_val = omemo_bundle_get_signed_pre_key(om_bundle_p, &signed_pre_key_id, &signed_pre_key_p, &signed_pre_key_len);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get the signed pre key from the bundle");
        goto cleanup;
    }
    ret_val = omemo_bundle_get_signature(om_bundle_p, &signature_p, &signature_len);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get the signature from the bundle");
        goto cleanup;
    }
    ret_val = omemo_bundle_get_identity_key(om_bundle_p, &identity_key_p, &identity_key_len);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get the public identity key from the bundle");
        goto cleanup;
    }

    pre_key_buf_p = axc_buf_create(pre_key_p, pre_key_len);
    signed_pre_key_buf_p = axc_buf_create(signed_pre_key_p, signed_pre_key_len);
    signature_buf_p = axc_buf_create(signature_p, signature_len);
    identity_key_buf_p = axc_buf_create(identity_key_p, identity_key_len);

    if (!pre_key_buf_p || !signed_pre_key_buf_p || !signature_buf_p || !identity_key_buf_p) {
        ret_val = LURCH_ERR;
        err_msg_dbg = g_strdup_printf("failed to create one of the buffers");
        goto cleanup;
    }

    ret_val = axc_session_from_bundle(pre_key_id, pre_key_buf_p,
                                      signed_pre_key_id, signed_pre_key_buf_p,
                                      signature_buf_p,
                                      identity_key_buf_p,
                                      &remote_addr,
                                      axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to create a session from a bundle");
        goto cleanup;
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }
    omemo_bundle_destroy(om_bundle_p);
    free(pre_key_p);
    free(signed_pre_key_p);
    free(signature_p);
    free(identity_key_p);
    axc_buf_free(pre_key_buf_p);
    axc_buf_free(signed_pre_key_buf_p);
    axc_buf_free(signature_buf_p);
    axc_buf_free(identity_key_buf_p);

    return ret_val;
}


bool Omemo::isEncryptedMessage(const QString& xmlNode)
{
    bool returnValue = false;

    QDomDocument d;
    if (d.setContent(xmlNode) == true)
    {
        QDomNodeList nodeList = d.elementsByTagName("message");
        if (!nodeList.isEmpty())
        {
            //qDebug() << "found msg";
            QDomNodeList encList = d.elementsByTagName("encrypted");
            if (!encList.isEmpty())
            {
                //qDebug() << "found enc";
                returnValue = true;
            }
        }
    }

    return returnValue;
}

//
// lurch_msg_encrypt_for_addrs
// lurch_bundle_request_cb
// lurch_bundle_request_do
// decrypt msg
// encrypt msg

