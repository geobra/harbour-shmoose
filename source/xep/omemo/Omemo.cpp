#include "Omemo.h"
#include "System.h"
#include "XmlProcessor.h"
#include "Settings.h"
#include "System.h"

#include <QDir>
#include <QDebug>

extern "C" {
#include "libomemo_crypto.h"
#include "libomemo_storage.h"
#include "axc_store.h"
}

#define LURCH_DB_SUFFIX "_db.sqlite"
#define LURCH_DB_NAME_OMEMO "omemo"
#define LURCH_DB_NAME_AXC "axc"

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
    free(uname_);
}

void Omemo::setupWithClient(Swift::Client* client)
{
    client_ = client;

    myBareJid_ = QString::fromStdString(client_->getJID().toBare().toString());
    uname_ = strdup(myBareJid_.toStdString().c_str());

    client_->onDataRead.connect(boost::bind(&Omemo::handleDataReceived, this, _1));

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
                // FIXME implement me!
                qDebug() << "requested someone else device list";
            }
        }
    }
}

void Omemo::ownDeviceListRequestHandler(QString fromJid, QString items)
{
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

    ret_val = devicelistProcess(dl_p);
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

int Omemo::devicelistProcess(omemo_devicelist * dl_in_p)
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
    db_fn_omemo = unameGetDbFn(uname_, (char*)LURCH_DB_NAME_OMEMO);

    purple_debug_info("lurch", "%s: processing devicelist from %s for %s\n", __func__, from, uname_);

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
}

