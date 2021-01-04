#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

/*
 * #######################################
 * ## This file is ported from lurch    ##
 * ## https://github.com/gkdr/lurch     ##
 * ##                                   ##
 * ## Thank you for all the great work! ##
 * #######################################
 *
 * As a technical decision, this file tries to be as close as possible to
 * the original lurch.c source. All the libpurple stuff is stripped away.
 * The comments on the headers are still the original ones, despite the
 * fact that some functions take now different parameters. Its done to be
 * easy diffable with future lurch releases.
 */

#include "Omemo.h"
#include "System.h"
#include "XmlProcessor.h"
#include "Settings.h"
#include "System.h"
#include "RawRequestWithFromJid.h"
#include "RawRequestBundle.h"

#include <QDir>
#include <QDomDocument>
#include <QDebug>

#include <stdlib.h>

extern "C" {
#include <purple.h>
#include "lurch_util.h"
#include "libomemo_storage.h"
#include "axc_store.h"
}

// see https://www.ietf.org/rfc/rfc3920.txt
#define JABBER_MAX_LEN_NODE 1023
#define JABBER_MAX_LEN_DOMAIN 1023
#define JABBER_MAX_LEN_BARE JABBER_MAX_LEN_NODE + JABBER_MAX_LEN_DOMAIN + 1

#define LURCH_ERR           -1000000
#define LURCH_ERR_NOMEM     -1000001
#define LURCH_ERR_NO_BUNDLE -1000010


#define LURCH_ERR_STRING_ENCRYPT "There was an error encrypting the message and it was not sent. " \
    "You can try again, or try to find the problem by looking at the debug log."
#define LURCH_ERR_STRING_DECRYPT "There was an error decrypting an OMEMO message addressed to this device. " \
    "See the debug log for details."

/*
 * https://xmpp.org/extensions/xep-0384.html
 *
 * 1. check if own id is in my device list. update if necessary. give read access to world
 * 2. check all contacts if they support omemo. subscribe to 'eu.siacs.conversations.axolotl.devicelist' (new: 'urn:xmpp:omemo:1:devices'). must cache that list
 * 3. create and publish bundle. give read access to world
 * 4. build a session (fetch other clients bundle information)
 * 5. encrypt and send the message
 * 6. receive and decrypt a message
 */

Omemo::Omemo(QObject *parent) : QObject(parent)
{
    // lurch_plugin_load

    set_omemo_dir(System::getOmemoPath().toStdString().c_str());

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

    //qDebug() << "Omemo::Omemo" << deviceListNodeName_;
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

    RawRequestWithFromJid::ref requestDeviceList = RawRequestWithFromJid::create(Swift::IQ::Get, jid.toBare(), deviceListRequestXml, client_->getIQRouter());
    requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1, _2));
    requestDeviceList->send();
}

void Omemo::lurch_addr_list_destroy_func(gpointer data) {
  lurch_addr * addr_p = (lurch_addr *) data;
  free(addr_p->jid);
  free(addr_p);
}

/**
 * Creates a queued msg.
 * Note that it just saves the pointers, so make sure they are not freed during
 * the lifetime of this struct and instead use the destroy function when done.
 *
 * @param om_msg_p Pointer to the omemo_message.
 * @param recipient_addr_l_p Pointer to the list of recipient addresses.
 * @param no_sess_l_p Pointer to the list that contains the addresses that do
 *                    not have sessions, i.e. for which bundles were requested.
 * @param cmsg_pp Will point to the pointer of the created queued msg struct.
 * @return 0 on success, negative on error.
 */

int Omemo::lurch_queued_msg_create(omemo_message * om_msg_p,
                                   GList * recipient_addr_l_p,
                                   GList * no_sess_l_p,
                                   lurch_queued_msg ** qmsg_pp) {
    int ret_val = 0;
    char * err_msg_dbg{nullptr};

    lurch_queued_msg * qmsg_p{nullptr};
    GHashTable * sess_handled_p{nullptr};

    qmsg_p = (lurch_queued_msg *)malloc(sizeof(lurch_queued_msg));
    if (!qmsg_p) {
        ret_val = LURCH_ERR_NOMEM;
        err_msg_dbg = g_strdup_printf("failed to malloc space for queued msg struct");
        goto cleanup;
    }

    sess_handled_p = g_hash_table_new(g_str_hash, g_str_equal);

    qmsg_p->om_msg_p = om_msg_p;
    qmsg_p->recipient_addr_l_p = recipient_addr_l_p;
    qmsg_p->no_sess_l_p = no_sess_l_p;
    qmsg_p->sess_handled_p = sess_handled_p;

    *qmsg_pp = qmsg_p;

cleanup:
    if (ret_val) {
        free(qmsg_p);
    }
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }
    return ret_val;
}

int Omemo::lurch_queued_msg_is_handled(const lurch_queued_msg * qmsg_p) {
  return (g_list_length(qmsg_p->no_sess_l_p) == g_hash_table_size(qmsg_p->sess_handled_p)) ? 1 : 0;
}

/**
 * Frees all the memory used by the queued msg.
 */
void Omemo::lurch_queued_msg_destroy(lurch_queued_msg * qmsg_p) {
  if (qmsg_p) {
    omemo_message_destroy(qmsg_p->om_msg_p);
    g_list_free_full(qmsg_p->recipient_addr_l_p, free);
    g_hash_table_destroy(qmsg_p->sess_handled_p);
    free(qmsg_p);
  }
}

char * Omemo::lurch_queue_make_key_string_s(const char * name, const char * device_id) {
  return g_strconcat(name, "/", device_id, NULL);
}

/**
 * Returns the db name, has to be g_free()d.
 *
 * @param uname The username.
 * @param which Either LURCH_DB_NAME_OMEMO or LURCH_DB_NAME_AXC
 * @return The path string.
 */
char* Omemo::unameGetDbFn(const char * uname, char * which)
{
    // impelements lurch_uname_get_db_fn

    // FIXME remove me! use lurch_util_uname_get_db_fn

    return g_strconcat(System::getOmemoPath().toStdString().c_str(), "/", uname, "_", which, LURCH_DB_SUFFIX, NULL);

}

/**
 * Does the first-time install of the axc DB.
 * As specified in OMEMO, it checks if the generated device ID already exists.
 * Therefore, it should be called at a point in time when other entries exist.
 *
 * @param uname The username.
 * @return 0 on success, negative on error.
 */
bool Omemo::axcPrepare(QString fromJid)
{
    // implements lurch_axc_prepare
    int ret_val = 0;

    char * err_msg_dbg = nullptr;
    axc_context * axc_ctx_p = nullptr;
    uint32_t device_id = 0;
    char * db_fn_omemo = nullptr;

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get init axc ctx");
        goto cleanup;
    }

    ret_val = axc_get_device_id(axc_ctx_p, &device_id);
    if (!ret_val) {
        // already installed
        goto cleanup;
    }

    db_fn_omemo = unameGetDbFn(fromJid.toStdString().c_str(), (char *)LURCH_DB_NAME_OMEMO);

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
 * Encrypts a data buffer, usually the omemo symmetric key, using axolotl.
 * Assumes a valid session already exists.
 *
 * @param recipient_addr_p Pointer to the lurch_addr of the recipient.
 * @param key_p Pointer to the key data.
 * @param key_len Length of the key data.
 * @param axc_ctx_p Pointer to the axc_context to use.
 * @param key_ct_pp Will point to a pointer to an axc_buf containing the key ciphertext on success.
 * @return 0 on success, negative on error
 */
int Omemo::lurch_key_encrypt(const lurch_addr * recipient_addr_p,
                             const uint8_t * key_p,
                             size_t key_len,
                             axc_context * axc_ctx_p,
                             axc_buf ** key_ct_buf_pp) {
  int ret_val = 0;
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
  axc_addr.device_id = (int32_t)recipient_addr_p->device_id;

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

/**
 * For each of the recipients, encrypts the symmetric key using the existing axc session,
 * then adds it to the omemo message.
 * If the session does not exist, the recipient is skipped.
 *
 * @param om_msg_p Pointer to the omemo message.
 * @param addr_l_p Pointer to the head of a list of the intended recipients' lurch_addrs.
 * @param axc_ctx_p Pointer to the axc_context to use.
 * @return 0 on success, negative on error.
 */
int Omemo::lurch_msg_encrypt_for_addrs(omemo_message * om_msg_p, GList * addr_l_p, axc_context * axc_ctx_p) {
    int ret_val = 0;
    char * err_msg_dbg{nullptr};

    GList * curr_l_p{nullptr};
    lurch_addr * curr_addr_p{nullptr};
    axc_address addr{};
    axc_buf * curr_key_ct_buf_p{nullptr};

    purple_debug_info("lurch", "%s: trying to encrypt key for %i devices\n", __func__, g_list_length(addr_l_p));

    for (curr_l_p = addr_l_p; curr_l_p; curr_l_p = curr_l_p->next) {
        curr_addr_p = (lurch_addr *) curr_l_p->data;
        addr.name = curr_addr_p->jid;
        addr.name_len = strnlen(addr.name, JABBER_MAX_LEN_BARE);
        addr.device_id = (int32_t)curr_addr_p->device_id;

        ret_val = axc_session_exists_initiated(&addr, axc_ctx_p);
        if (ret_val < 0) {
            err_msg_dbg = g_strdup_printf("failed to check if session exists, aborting");
            goto cleanup;
        } else if (!ret_val) {
            continue;
        } else {
            ret_val = lurch_key_encrypt(curr_addr_p,
                                        omemo_message_get_key(om_msg_p),
                                        omemo_message_get_key_len(om_msg_p),
                                        axc_ctx_p,
                                        &curr_key_ct_buf_p);
            if (ret_val) {
                err_msg_dbg = g_strdup_printf("failed to encrypt key for %s:%i", curr_addr_p->jid, curr_addr_p->device_id);
                goto cleanup;
            }

            ret_val = omemo_message_add_recipient(om_msg_p,
                                                  curr_addr_p->device_id,
                                                  axc_buf_get_data(curr_key_ct_buf_p),
                                                  axc_buf_get_len(curr_key_ct_buf_p));
            if (ret_val) {
                err_msg_dbg = g_strdup_printf("failed to add recipient to omemo msg");
                goto cleanup;
            }

            axc_buf_free(curr_key_ct_buf_p);
            curr_key_ct_buf_p = nullptr;
        }
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }
    axc_buf_free(curr_key_ct_buf_p);

    return ret_val;
}

/**
 * Collects the information needed for a bundle and publishes it.
 *
 * @param uname The username.
 * @param js_p Pointer to the connection to use for publishing.
 */
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

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
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

/**
 * Creates an axc session from a received bundle.
 *
 * @param uname The own username.
 * @param from The sender of the bundle.
 * @param items_p The bundle update as received in the PEP request handler.
 */
int Omemo::bundleCreateSession(const char* from, const std::string& items, axc_context * axc_ctx_p)
{
    // lurch_bundle_create_session
    int ret_val{0};
    char * err_msg_dbg{nullptr};

    //int len{};
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
    remote_addr.device_id = (int32_t)omemo_bundle_get_device_id(om_bundle_p);

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

/**
 * Wraps the omemo_message_export_encrypted message, so that it is called with the same options throughout.
 */
int Omemo::lurch_export_encrypted(omemo_message * om_msg_p, char ** xml_pp) {
  return omemo_message_export_encrypted(om_msg_p, OMEMO_ADD_MSG_EME, xml_pp);
}

/**
 * Implements JabberIqCallback.
 * Callback for a bundle request.
 */
void Omemo::bundleRequestCb(const std::string& fromStr, JabberIqType type, const std::string& idStr,
                            const std::string& packet_p, lurch_queued_msg * qmsg_p) {

    // Implements lurch_bundle_request_cb
  int ret_val{0};
  char * err_msg_conv{nullptr};
  char * err_msg_dbg{nullptr};

  //char * uname = (void *) 0;
  //char ** split = (void *) 0;
  const char * device_id_str{nullptr};;
  axc_address addr{};
  axc_context * axc_ctx_p{nullptr};
  char * recipient{nullptr};
  //xmlnode * pubsub_node_p = (void *) 0;
  //xmlnode * items_node_p = (void *) 0;
  int msg_handled{0};
  char * addr_key{nullptr};
  char * msg_xml{nullptr};
  //xmlnode * msg_node_p = (void *) 0;
  //lurch_queued_msg * qmsg_p = (lurch_queued_msg *) data_p;

  const char * from = fromStr.c_str();
  std::string items{};

  //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(js_p->gc)));
  recipient = omemo_message_get_recipient_name_bare(qmsg_p->om_msg_p);

  if (!from) {
    // own user
    from = uname_;
  }

  //split = g_strsplit(id, "#", 3);
  device_id_str = idStr.c_str();

  purple_debug_info("lurch", "%s: %s received bundle update from %s:%s\n", __func__, uname_, from, device_id_str);

  addr.name = from;
  addr.name_len = strnlen(from, JABBER_MAX_LEN_BARE);
  addr.device_id = (int32_t)strtol(device_id_str, nullptr, 10);

  //ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
  ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
  if (ret_val) {
    err_msg_dbg = "failed to get axc ctx";
    goto cleanup;
  }

  if (type == JABBER_IQ_ERROR) {
    err_msg_conv = g_strdup_printf("The device %s owned by %s does not have a bundle and will be skipped. "
                                   "The owner should fix this, or remove the device from the list.", device_id_str, from);

  } else {
#if 0
    pubsub_node_p = xmlnode_get_child(packet_p, "pubsub");
    if (!pubsub_node_p) {
      ret_val = LURCH_ERR;
      err_msg_dbg = "no <pubsub> node in response";
      goto cleanup;
    }
#endif

    //items_node_p = xmlnode_get_child(pubsub_node_p, "items");
    QString qItems = XmlProcessor::getChildFromNode("items", QString::fromStdString(packet_p));
    if (qItems.isEmpty() == true) {
      ret_val = LURCH_ERR;
      err_msg_dbg = "no <items> node in response";
      goto cleanup;
    }

    ret_val = axc_session_exists_initiated(&addr, axc_ctx_p);
    if (!ret_val) {
      //ret_val = lurch_bundle_create_session(uname_, from, items_node_p, axc_ctx_p);
      ret_val = bundleCreateSession(from, qItems.toStdString(), axc_ctx_p);
      if (ret_val) {
        err_msg_dbg = "failed to create a session";
        goto cleanup;
      }
    } else if (ret_val < 0) {
      err_msg_dbg = "failed to check if session exists";
      goto cleanup;
    }
  }

  addr_key = lurch_queue_make_key_string_s(from, device_id_str);
  if (!addr_key) {
    err_msg_dbg = "failed to make a key string";
    ret_val = LURCH_ERR;
    goto cleanup;
  }

  (void) g_hash_table_replace(qmsg_p->sess_handled_p, addr_key, addr_key);

  if (lurch_queued_msg_is_handled(qmsg_p)) {
    msg_handled = 1;
  }

  if (msg_handled) {
    ret_val = lurch_msg_encrypt_for_addrs(qmsg_p->om_msg_p, qmsg_p->recipient_addr_l_p, axc_ctx_p);
    if (ret_val) {
      err_msg_dbg = "failed to encrypt the symmetric key";
      goto cleanup;
    }

    ret_val = lurch_export_encrypted(qmsg_p->om_msg_p, &msg_xml);
    if (ret_val) {
      err_msg_dbg = "failed to export the message to xml";
      goto cleanup;
    }

#if 0
    msg_node_p = xmlnode_from_str(msg_xml, -1);
    if (!msg_node_p) {
      err_msg_dbg = "failed to parse xml from string";
      ret_val = LURCH_ERR;
      goto cleanup;
    }
#endif

    emit rawMessageStanzaForSending(QString::fromLatin1(msg_xml));

    purple_debug_info("lurch", "sending encrypted msg\n");
    //purple_signal_emit(purple_plugins_find_with_id("prpl-jabber"), "jabber-sending-xmlnode", js_p->gc, &msg_node_p);

    lurch_queued_msg_destroy(qmsg_p);
  }

cleanup:
  if (err_msg_conv) {
    //purple_conv_present_error(recipient, purple_connection_get_account(js_p->gc), err_msg_conv);
    // FIXME show error to user!
    std::cout << err_msg_conv << std::endl;
    g_free(err_msg_conv);
  }
  if (err_msg_dbg) {
    //purple_conv_present_error(recipient, purple_connection_get_account(js_p->gc), LURCH_ERR_STRING_ENCRYPT);
    //  FIXME show error to user!
    purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
  }

  //free(uname);
  //g_strfreev(split);

  free(addr_key);
  free(recipient);
  free(msg_xml);
  //if (msg_node_p) {
  //  xmlnode_free(msg_node_p);
  //}
}

/**
 * Requests a bundle.
 *
 * @param js_p Pointer to the JabberStream to use.
 * @param to The recipient of this request.
 * @param device_id The ID of the device whose bundle is needed.
 * @param qmsg_p Pointer to the queued message waiting on (at least) this bundle.
 * @return 0 on success, negative on error.
 */
int Omemo::bundleRequestDo(const char * to, uint32_t device_id, lurch_queued_msg * qmsg_p) {
    // lurch_bundle_request_do

    /* <iq type='get' to='xxx@jabber.ccc.de' id='xxx@jabber.ccc.de#508164373#-1085008789'>
        <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node='eu.siacs.conversations.axolotl.bundles:508164373' max_items='1'/>
        </pubsub>
       </iq>
    */

    Swift::IDGenerator idGenerator;

    std::string toJid{to};
    //std::string reqId = toJid + "#" + std::to_string(device_id) + "#" + idGenerator.generateID();
    std::string bundleId = std::to_string(device_id);

    // gen the payload
    char* cBundlePep{nullptr};
    int retVal = omemo_bundle_get_pep_node_name(device_id, &cBundlePep);
    if (retVal != 0)
    {
        qDebug() << "failed to get bundlelist pep node name";
    }
    else
    {
        const std::string pubsubXml =
                "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(cBundlePep) +"' max_items='1'/></pubsub>";
        free(cBundlePep);

        RawRequestBundle::ref publishPep = RawRequestBundle::create(Swift::IQ::Get,
                                                                                toJid,
                                                                                pubsubXml,
                                                                                client_->getIQRouter(),
                                                                                bundleId,
                                                                                qmsg_p
                                                                            );

        publishPep->onResponse.connect(boost::bind(&Omemo::requestBundleHandler, this, _1, _2, _3, _4));
        publishPep->send();
    }


#if 0
    int ret_val = 0;

    JabberIq * jiq_p{nullptr};
    xmlnode * pubsub_node_p{nullptr};
    char * device_id_str{nullptr};
    char * rand_str{nullptr};
    char * req_id{nullptr};
    char * bundle_node_name{nullptr};
    xmlnode * items_node_p{nullptr};

    purple_debug_info("lurch", "%s: %s is requesting bundle from %s:%i\n", __func__,
                      uname_, to, device_id);

    jiq_p = jabber_iq_new(js_p, JABBER_IQ_GET);
    xmlnode_set_attrib(jiq_p->node, "to", to);

    pubsub_node_p = xmlnode_new_child(jiq_p->node, "pubsub");
    xmlnode_set_namespace(pubsub_node_p, "http://jabber.org/protocol/pubsub");

    device_id_str = g_strdup_printf("%i", device_id);
    rand_str = g_strdup_printf("%i", g_random_int());
    req_id = g_strconcat(to, "#", device_id_str, "#", rand_str, NULL);

    ret_val = omemo_bundle_get_pep_node_name(device_id, &bundle_node_name);
    if (ret_val) {
        purple_debug_error("lurch", "%s: failed to get bundle pep node name for %s:%i\n", __func__, to, device_id);
        goto cleanup;
    }

    items_node_p = xmlnode_new_child(pubsub_node_p, "items");
    xmlnode_set_attrib(items_node_p, "node", bundle_node_name);
    xmlnode_set_attrib(items_node_p, "max_items", "1");

    jabber_iq_set_id(jiq_p, req_id);
    jabber_iq_set_callback(jiq_p, lurch_bundle_request_cb, qmsg_p);

    jabber_iq_send(jiq_p);

    purple_debug_info("lurch", "%s: ...request sent\n", __func__);

cleanup:
    g_free(device_id_str);
    g_free(rand_str);
    g_free(req_id);
    g_free(bundle_node_name);

    return ret_val;
#endif

    return 0;
}

void Omemo::requestBundleHandler(const Swift::JID& jid, const std::string& bundleId, lurch_queued_msg* qMsg, const std::string& str)
{
    // str has pubsub as root node. The cb wants it to be child of iq...
    std::string bundleResponse = "<iq>" + str + "</iq>";

    //std::cout << jid.toString() << ", " << bundleId << ", " << bundleResponse << std::endl;
    bundleRequestCb(jid.toBare().toString(), JABBER_IQ_SET, bundleId, bundleResponse, qMsg);
}


void Omemo::pepBundleForKeytransport(const std::string from, const std::string &items)
{
    // lurch_pep_bundle_for_keytransport
    int ret_val{0};
    char * err_msg_dbg = nullptr;

    //char * uname = (void *) 0;
    axc_context * axc_ctx_p{nullptr};
    uint32_t own_id{0};
    omemo_message * msg_p{nullptr};
    axc_address addr{};
    lurch_addr laddr{};
    axc_buf * key_ct_buf_p{};
    char * msg_xml{nullptr};
    //xmlnode * msg_node_p{nullptr};
    //void * jabber_handle_p = purple_plugins_find_with_id("prpl-jabber");

    //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(js_p->gc)));

    // items string starts at the <pubsub... > level, strip that away.
    QString qItems = XmlProcessor::getChildFromNode("items", QString::fromStdString(items));

    addr.name = from.c_str();
    //addr.name_len = strnlen(from, JABBER_MAX_LEN_BARE);
    addr.name_len = from.size();

    //addr.device_id = lurch_bundle_name_get_device_id(xmlnode_get_attrib(items_p, "node"));
    QString nodesStr = XmlProcessor::getContentInTag("items", "node", qItems);

    QStringList nodesStrList = nodesStr.split(":");
    if (nodesStrList.size() == 2)
    {
        addr.device_id = atoi(nodesStrList.at(1).toStdString().c_str());
    }
    else
    {
        // FIXME show error to user
        ret_val = LURCH_ERR;
        err_msg_dbg = "no <items> node in response";
        goto cleanup;
    }

    purple_debug_info("lurch", "%s: %s received bundle from %s:%i\n", __func__, uname_, from.c_str(), addr.device_id);

    laddr.jid = g_strndup(addr.name, addr.name_len);
    laddr.device_id = (uint32_t)addr.device_id;

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to init axc ctx");
        goto cleanup;
    }

    // make sure it's gonna be a pre_key_message
    ret_val = axc_session_delete(addr.name, (uint32_t)addr.device_id, axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to delete possibly existing session");
        goto cleanup;
    }

    ret_val = bundleCreateSession(from.c_str(), qItems.toStdString(), axc_ctx_p);
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

    ret_val = lurch_key_encrypt(&laddr,
                         omemo_message_get_key(msg_p),
                         omemo_message_get_key_len(msg_p),
                         axc_ctx_p,
                         &key_ct_buf_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to encrypt key for %s:%i", addr.name, addr.device_id);
        goto cleanup;
    }

    ret_val = omemo_message_add_recipient(msg_p,
                                          (uint32_t)addr.device_id,
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

    //purple_signal_emit(jabber_handle_p, "jabber-sending-xmlnode", js_p->gc, &msg_node_p);
    emit rawMessageStanzaForSending(QString::fromLatin1(msg_xml));

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

/**
 * Processes a devicelist by updating the database with it.
 *
 * @param uname The username.
 * @param dl_in_p Pointer to the incoming devicelist.
 * @param js_p Pointer to the JabberStream.
 * @return 0 on success, negative on error.
 */
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

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
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

/**
 * A JabberPEPHandler function.
 * Is used to handle the own devicelist and also perform install-time functions.
 */
void Omemo::ownDeviceListRequestHandler(QString items)
{
    // lurch_pep_own_devicelist_request_handler
    // implements 4.3 Announcing support -> device list

    int ret_val = 0;
    char * err_msg_dbg{nullptr};

    //int len = 0;
    int install =0;
    axc_context * axc_ctx_p{nullptr};
    uint32_t own_id = 0;
    int needs_publishing = 1;
    omemo_devicelist * dl_p{nullptr};
    char * dl_xml{nullptr};

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

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
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

#if 0
// FIXME implement me!
/**
 * A JabberPEPHandler function.
 * On receiving a devicelist PEP updates the database entry.
 */
static void lurch_pep_devicelist_event_handler(JabberStream * js_p, const char * from, xmlnode * items_p) {
  int ret_val = 0;
  int len = 0;
  char * err_msg_dbg = (void *) 0;

  char * uname = (void *) 0;
  omemo_devicelist * dl_in_p = (void *) 0;

  uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(js_p->gc)));
  if (!strncmp(uname, from, strnlen(uname, JABBER_MAX_LEN_BARE))) {
    //own devicelist is dealt with in own handler
    lurch_pep_own_devicelist_request_handler(js_p, from, items_p);
    goto cleanup;
  }

  purple_debug_info("lurch", "%s: %s received devicelist update from %s\n", __func__, uname, from);

  ret_val = omemo_devicelist_import(xmlnode_to_str(items_p, &len), from, &dl_in_p);
  if (ret_val) {
    err_msg_dbg = g_strdup_printf("failed to import devicelist");
    goto cleanup;
  }

  ret_val = lurch_devicelist_process(uname, dl_in_p, js_p);
  if(ret_val) {
    err_msg_dbg = g_strdup_printf("failed to process devicelist");
    goto cleanup;
  }

cleanup:
  if (err_msg_dbg) {
    purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
    g_free(err_msg_dbg);
  }
  g_free(uname);
  omemo_devicelist_destroy(dl_in_p);
}
#endif

#if 0
// done in Omemo setupWithClient
/**
 * Set as callback for the "account connected" signal.
 * Requests the own devicelist, as that requires an active connection (as
 * opposed to just registering PEP handlers).
 * Also inits the msg queue hashtable.
 */
static void lurch_account_connect_cb(PurpleAccount * acc_p) {
  int ret_val = 0;

  char * uname = (void *) 0;
  JabberStream * js_p = (void *) 0;
  char * dl_ns = (void *) 0;

 // purple_account_set_bool(acc_p, LURCH_ACC_SETTING_INITIALIZED, FALSE);

  js_p = purple_connection_get_protocol_data(purple_account_get_connection(acc_p));

  if (strncmp(purple_account_get_protocol_id(acc_p), JABBER_PROTOCOL_ID, strlen(JABBER_PROTOCOL_ID))) {
    return;
  }

  ret_val = omemo_devicelist_get_pep_node_name(&dl_ns);
  if (ret_val) {
    purple_debug_error("lurch", "%s: %s (%i)\n", __func__, "failed to get devicelist pep node name", ret_val);
    goto cleanup;
  }
  uname = lurch_uname_strip(purple_account_get_username(acc_p));
  jabber_pep_request_item(js_p, uname, dl_ns, (void *) 0, lurch_pep_own_devicelist_request_handler);

cleanup:
  g_free(uname);
  free(dl_ns);
}
#endif

/**
 * For a list of lurch_addrs, checks which ones do not have an active session.
 * Note that the structs are not copied, the returned list is just a subset
 * of the pointers of the input list.
 *
 * @param addr_l_p A list of pointers to lurch_addr structs.
 * @param axc_ctx_p The axc_context to use.
 * @param no_sess_l_pp Will point to a list that contains pointers to those
 *                     addresses that do not have a session.
 * @return 0 on success, negative on error.
 */
int Omemo::lurch_axc_sessions_exist(GList * addr_l_p, axc_context * axc_ctx_p, GList ** no_sess_l_pp){
  int ret_val = 0;

  GList * no_sess_l_p{nullptr};

  GList * curr_p;
  lurch_addr * curr_addr_p;
  axc_address curr_axc_addr{};
  for (curr_p = addr_l_p; curr_p; curr_p = curr_p->next) {
    curr_addr_p = (lurch_addr *) curr_p->data;

    curr_axc_addr.name = curr_addr_p->jid;
    curr_axc_addr.name_len = strnlen(curr_axc_addr.name, JABBER_MAX_LEN_BARE);
    curr_axc_addr.device_id = (int32_t)curr_addr_p->device_id;

    ret_val = axc_session_exists_initiated(&curr_axc_addr, axc_ctx_p);
    if (ret_val < 0) {
      purple_debug_error("lurch", "%s: %s (%i)\n", __func__, "failed to see if session exists", ret_val);
      goto cleanup;
    } else if (ret_val > 0) {
      ret_val = 0;
      continue;
    } else {
      no_sess_l_p = g_list_prepend(no_sess_l_p, curr_addr_p);
      ret_val = 0;
    }
  }

  *no_sess_l_pp = no_sess_l_p;

cleanup:
  return ret_val;
}

/**
 * Adds an omemo devicelist to a GList of lurch_addrs.
 *
 * @param addrs_p Pointer to the list to add to. Remember NULL is a valid GList *.
 * @param dl_p Pointer to the omemo devicelist to add.
 * @param exclude_id_p Pointer to an ID that is not to be added. Useful when adding the own devicelist. Can be NULL.
 * @return Pointer to the updated GList on success, NULL on error.
 */
GList* Omemo::lurch_addr_list_add(GList * addrs_p, const omemo_devicelist * dl_p, const uint32_t * exclude_id_p) {
    int ret_val = 0;

    GList * new_l_p = addrs_p;
    GList * dl_l_p{nullptr};
    GList * curr_p{nullptr};
    lurch_addr curr_addr = {};
    uint32_t curr_id = 0;
    lurch_addr * temp_addr_p{nullptr};

    curr_addr.jid = g_strdup(omemo_devicelist_get_owner(dl_p));

    dl_l_p = omemo_devicelist_get_id_list(dl_p);

    for (curr_p = dl_l_p; curr_p; curr_p = curr_p->next) {
        curr_id = omemo_devicelist_list_data(curr_p);
        if (exclude_id_p && *exclude_id_p == curr_id) {
            continue;
        }

        curr_addr.device_id = curr_id;

        temp_addr_p = (lurch_addr*)malloc(sizeof(lurch_addr));
        if (!temp_addr_p) {
            ret_val = LURCH_ERR_NOMEM;
            goto cleanup;
        }

        memcpy(temp_addr_p, &curr_addr, sizeof(lurch_addr));

        new_l_p = g_list_prepend(new_l_p, temp_addr_p);
    }

cleanup:
    g_list_free_full(dl_l_p, free);

    if (ret_val) {
        g_list_free_full(new_l_p, lurch_addr_list_destroy_func);
        return nullptr;
    } else {
        return new_l_p;
    }
}

/**
 * Does the final steps of encrypting the message.
 * If all devices have sessions, does the actual encrypting.
 * If not, saves it and sends bundle request to the missing devices so that the message can be sent at a later time.
 *
 * Note that if msg_stanza_pp points to NULL, both om_msg_p and addr_l_p must not be freed by the calling function.
 *
 * @param js_p          Pointer to the JabberStream to use.
 * @param axc_ctx_p     Pointer to the axc_context to use.
 * @param om_msg_p      Pointer to the omemo message.
 * @param addr_l_p      Pointer to a GList of lurch_addr structs that are supposed to receive the message.
 * @param msg_stanza_pp Pointer to the pointer to the <message> stanza.
 *                      Is either changed to point to the encrypted message, or to NULL if the message is to be sent later.
 * @return 0 on success, negative on error.
 *
 */
std::string Omemo::msgFinalizeEncryption(axc_context * axc_ctx_p, omemo_message * om_msg_p, GList * addr_l_p, const std::string& msg_stanza_pp) {
    (void)msg_stanza_pp;
    std::string finalEncMsg{};
    int ret_val = 0;
    char * err_msg_dbg{nullptr};

    GList * no_sess_l_p{nullptr};
    char * xml{nullptr};
    //xmlnode * temp_node_p{nullptr};
    lurch_queued_msg * qmsg_p{nullptr};
    GList * curr_item_p{nullptr};
    lurch_addr curr_addr{};

    ret_val = lurch_axc_sessions_exist(addr_l_p, axc_ctx_p, &no_sess_l_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to check if sessions exist");
        goto cleanup;
    }

    if (!no_sess_l_p) {
        ret_val = lurch_msg_encrypt_for_addrs(om_msg_p, addr_l_p, axc_ctx_p);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to encrypt symmetric key for addrs");
            goto cleanup;
        }

        ret_val = lurch_export_encrypted(om_msg_p, &xml);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to export omemo msg to xml");
            goto cleanup;
        }

        omemo_message_destroy(om_msg_p);
        //temp_node_p = xmlnode_from_str(xml, -1);
        //*msg_stanza_pp = temp_node_p;
        finalEncMsg = std::string(xml);
    } else {
        ret_val = lurch_queued_msg_create(om_msg_p, addr_l_p, no_sess_l_p, &qmsg_p);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to create queued message");
            goto cleanup;
        }

        for (curr_item_p = no_sess_l_p; curr_item_p; curr_item_p = curr_item_p->next) {
            curr_addr.jid = ((lurch_addr *)curr_item_p->data)->jid;
            curr_addr.device_id = ((lurch_addr *)curr_item_p->data)->device_id;

            purple_debug_info("lurch", "%s: %s has device without session %i, requesting bundle\n", __func__, curr_addr.jid, curr_addr.device_id);

/*
            lurch_bundle_request_do(js_p,
                                    curr_addr.jid,
                                    curr_addr.device_id,
                                    qmsg_p);
*/
            bundleRequestDo(curr_addr.jid, curr_addr.device_id, qmsg_p);
        }
        //*msg_stanza_pp = (void *) 0;
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
        //*msg_stanza_pp = (void *) 0;
    }
    if (!qmsg_p || ret_val) {
        free(qmsg_p);
    }

    free(xml);

    return finalEncMsg;
}

/**
 * Set as callback for the "sending xmlnode" signal.
 * Encrypts the message body, if applicable.
 */
std::string Omemo::messageEncryptIm(const std::string msg_stanza_pp) {
    // lurch_message_encrypt_im
    int ret_val = 0;
    char * err_msg_dbg{nullptr};
    //int len = 0;

    //PurpleAccount * acc_p{nullptr};
    //char * uname{nullptr};
    char * db_fn_omemo{nullptr};
    const char * to{nullptr};
    omemo_devicelist * dl_p{nullptr};
    GList * recipient_dl_p{nullptr};
    omemo_devicelist * user_dl_p{nullptr};
    GList * own_dl_p{nullptr};
    axc_context * axc_ctx_p{nullptr};
    uint32_t own_id = 0;
    omemo_message * msg_p{nullptr};
    GList * addr_l_p{nullptr};
    char * recipient{nullptr};
    char * tempxml{nullptr};

    std::string finalMsg{};

    //recipient = jabber_get_bare_jid(xmlnode_get_attrib(*msg_stanza_pp, "to"));
    recipient = strdup(XmlProcessor::getContentInTag("message", "to", QString::fromStdString(msg_stanza_pp)).toStdString().c_str());

    //acc_p = purple_connection_get_account(gc_p);
    //uname = lurch_uname_strip(purple_account_get_username(acc_p));
    db_fn_omemo = unameGetDbFn(uname_, (char*)LURCH_DB_NAME_OMEMO);

    ret_val = omemo_storage_chatlist_exists(recipient, db_fn_omemo);
    if (ret_val < 0) {
        err_msg_dbg = g_strdup_printf("failed to look up %s in DB %s", recipient, db_fn_omemo);
        goto cleanup;
    } else if (ret_val == 1) {
        purple_debug_info("lurch", "%s: %s is on blacklist, skipping encryption\n", __func__, recipient);
        goto cleanup;
    }

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get axc ctx for %s", uname_);
        goto cleanup;
    }

    ret_val = axc_get_device_id(axc_ctx_p, &own_id);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get device id");
        goto cleanup;
    }
    //tempxml = xmlnode_to_str(*msg_stanza_pp, &len);
    tempxml = strdup(msg_stanza_pp.c_str());
    ret_val = omemo_message_prepare_encryption(tempxml, own_id, &crypto, OMEMO_STRIP_ALL, &msg_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to construct omemo message");
        goto cleanup;
    }

    to = omemo_message_get_recipient_name_bare(msg_p);

    // determine if recipient is omemo user
    ret_val = omemo_storage_user_devicelist_retrieve(to, db_fn_omemo, &dl_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to retrieve devicelist for %s", to);
        goto cleanup;
    }

    free(tempxml);
    ret_val = omemo_devicelist_export(dl_p, &tempxml);
    if(ret_val) {
        err_msg_dbg = g_strdup_printf("failed to export devicelist for %s", to);
        goto cleanup;
    }
    purple_debug_info("lurch", "retrieved devicelist for %s:\n%s\n", to, tempxml);

    recipient_dl_p = omemo_devicelist_get_id_list(dl_p);
    if (!recipient_dl_p) {
        ret_val = axc_session_exists_any(to, axc_ctx_p);
        if (ret_val < 0) {
            err_msg_dbg = g_strdup_printf("failed to check if session exists for %s in %s's db\n", to, uname_);
            goto cleanup;
        } else if (ret_val == 1) {
            //purple_conv_present_error(recipient, purple_connection_get_account(gc_p), "Even though an encrypted session exists, the recipient's devicelist is empty."
            //                                                                          "The user probably uninstalled OMEMO, so you can add this conversation to the blacklist.");
            qDebug() << "FIXME show to user: "
                     << "Even though an encrypted session exists, the recipient's devicelist is empty. "
                     << "The user probably uninstalled OMEMO, so you can add this conversation to the blacklist.";
        } else {
            goto cleanup;
        }
    }

    ret_val = omemo_storage_user_devicelist_retrieve(uname_, db_fn_omemo, &user_dl_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to retrieve devicelist for %s", uname_);
        goto cleanup;
    }
    omemo_devicelist_export(user_dl_p, &tempxml);
    purple_debug_info("lurch", "retrieved own devicelist:\n%s\n", tempxml);
    own_dl_p = omemo_devicelist_get_id_list(user_dl_p);
    if (!own_dl_p) {
        err_msg_dbg = g_strdup_printf("no own devicelist");
        goto cleanup;
    }

    addr_l_p = lurch_addr_list_add(addr_l_p, user_dl_p, &own_id);
    if (g_strcmp0(uname_, to)) {
        addr_l_p = lurch_addr_list_add(addr_l_p, dl_p, nullptr);
    }

    //ret_val = lurch_msg_finalize_encryption(purple_connection_get_protocol_data(gc_p), axc_ctx_p, msg_p, addr_l_p, msg_stanza_pp);
    finalMsg = msgFinalizeEncryption(axc_ctx_p, msg_p, addr_l_p, msg_stanza_pp);
    if (finalMsg.empty() == true) {
        err_msg_dbg = g_strdup_printf("failed to finalize omemo message");
        goto cleanup;
    }

cleanup:
    if (err_msg_dbg) {
        //purple_conv_present_error(recipient, purple_connection_get_account(gc_p), LURCH_ERR_STRING_ENCRYPT);
        // FIXME present to user!
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
        //*msg_stanza_pp = nullptr;
    }
    if (ret_val) {
        omemo_message_destroy(msg_p);
        g_list_free_full(addr_l_p, lurch_addr_list_destroy_func);
    }
    free(recipient);
    //free(uname);
    free(db_fn_omemo);
    omemo_devicelist_destroy(dl_p);
    g_list_free_full(recipient_dl_p, free);
    omemo_devicelist_destroy(user_dl_p);
    g_list_free_full(own_dl_p, free);
    axc_context_destroy_all(axc_ctx_p);
    free(tempxml);

    return finalMsg;
}

#if 0
// FIXME implemt group chat encryption
static void lurch_message_encrypt_groupchat(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp) {
  int ret_val = 0;
  char * err_msg_dbg = (void *) 0;
  int len;

  char * uname = (void *) 0;
  char * db_fn_omemo = (void *) 0;
  axc_context * axc_ctx_p = (void *) 0;
  uint32_t own_id = 0;
  char * tempxml = (void *) 0;
  omemo_message * om_msg_p = (void *) 0;
  omemo_devicelist * user_dl_p = (void *) 0;
  GList * addr_l_p = (void *) 0;
  PurpleConversation * conv_p = (void *) 0;
  PurpleConvChat * chat_p = (void *) 0;
  JabberChat * muc_p = (void *) 0;
  JabberChatMember * curr_muc_member_p = (void *) 0;
  xmlnode * body_node_p = (void *) 0;
  GList * curr_item_p = (void *) 0;
  char * curr_muc_member_jid = (void *) 0;
  omemo_devicelist * curr_dl_p = (void *) 0;

  const char * to = xmlnode_get_attrib(*msg_stanza_pp, "to");

  uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(gc_p)));
  db_fn_omemo = lurch_uname_get_db_fn(uname, LURCH_DB_NAME_OMEMO);

  ret_val = omemo_storage_chatlist_exists(to, db_fn_omemo);
  if (ret_val < 0) {
    err_msg_dbg = g_strdup_printf("failed to access db %s", db_fn_omemo);
    goto cleanup;
  } else if (ret_val == 0) {
    goto cleanup;
  }

  ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
  if (ret_val) {
    err_msg_dbg = g_strdup_printf("failed to get axc ctx for %s", uname);
    goto cleanup;
  }

  ret_val = axc_get_device_id(axc_ctx_p, &own_id);
  if (ret_val) {
    err_msg_dbg = g_strdup_printf("failed to get device id");
    goto cleanup;
  }
  tempxml = xmlnode_to_str(*msg_stanza_pp, &len);
  ret_val = omemo_message_prepare_encryption(tempxml, own_id, &crypto, OMEMO_STRIP_ALL, &om_msg_p);
  if (ret_val) {
    err_msg_dbg = g_strdup_printf("failed to construct omemo message");
    goto cleanup;
  }

  ret_val = omemo_storage_user_devicelist_retrieve(uname, db_fn_omemo, &user_dl_p);
  if (ret_val) {
    err_msg_dbg = g_strdup_printf("failed to retrieve devicelist for %s", uname);
    goto cleanup;
  }

  addr_l_p = lurch_addr_list_add(addr_l_p, user_dl_p, &own_id);

  conv_p = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, to, purple_connection_get_account(gc_p));
  if (!conv_p) {
    err_msg_dbg = g_strdup_printf("could not find groupchat %s", to);
    goto cleanup;
  }

  chat_p = purple_conversation_get_chat_data(conv_p);
  muc_p = jabber_chat_find_by_conv(conv_p);
  if (!muc_p) {
    err_msg_dbg = g_strdup_printf("could not find muc struct for groupchat %s", to);
    goto cleanup;
  }

  for (curr_item_p = g_hash_table_get_values(muc_p->members); curr_item_p; curr_item_p = curr_item_p->next) {
    curr_muc_member_p = (JabberChatMember *) curr_item_p->data;
    curr_muc_member_jid = jabber_get_bare_jid(curr_muc_member_p->jid);

    if (!curr_muc_member_jid) {
      err_msg_dbg = g_strdup_printf("Could not find the JID for %s - the channel needs to be non-anonymous!", curr_muc_member_p->handle);
      purple_conv_present_error(purple_conversation_get_name(conv_p), purple_connection_get_account(gc_p), err_msg_dbg);
      g_free(err_msg_dbg);
      err_msg_dbg = (void *) 0;
      continue;
    }

    // libpurple (rightly) assumes that in MUCs the message will come back anyway so it's not written to the chat
    // but encrypting and decrypting for yourself should not be done with the double ratchet, so the own device is skipped
    // and the typed message written to the chat window manually without sending
    if (!g_strcmp0(curr_muc_member_jid, uname)) {
      body_node_p = xmlnode_get_child(*msg_stanza_pp, "body");

      purple_conv_chat_write(chat_p, curr_muc_member_p->handle, xmlnode_get_data(body_node_p), PURPLE_MESSAGE_SEND, time((void *) 0));
      continue;
    }

    ret_val = omemo_storage_user_devicelist_retrieve(curr_muc_member_jid, db_fn_omemo, &curr_dl_p);
    if (ret_val) {
      err_msg_dbg = g_strdup_printf("Could not retrieve the devicelist for %s from %s", curr_muc_member_jid, db_fn_omemo);
      goto cleanup;
    }

    if (omemo_devicelist_is_empty(curr_dl_p)) {
      err_msg_dbg = g_strdup_printf("User %s is no OMEMO user (does not have a devicelist). "
                                    "This user cannot read any incoming encrypted messages and will send his own messages in the clear!",
                                    curr_muc_member_jid);
      purple_conv_present_error(purple_conversation_get_name(conv_p), purple_connection_get_account(gc_p), err_msg_dbg);
      g_free(err_msg_dbg);
      err_msg_dbg = (void *) 0;
      continue;
    }

    addr_l_p = lurch_addr_list_add(addr_l_p, curr_dl_p, (void *) 0);
    omemo_devicelist_destroy(curr_dl_p);
    curr_dl_p = (void *) 0;
  }

  ret_val = lurch_msg_finalize_encryption(purple_connection_get_protocol_data(gc_p), axc_ctx_p, om_msg_p, addr_l_p, msg_stanza_pp);
  if (ret_val) {
    err_msg_dbg = g_strdup_printf("failed to finalize msg");
    goto cleanup;
  }

  //TODO: properly handle this instead of removing the body completely, necessary for full EME support
  body_node_p = xmlnode_get_child(*msg_stanza_pp, "body");
  xmlnode_free(body_node_p);

cleanup:
  if (err_msg_dbg) {
    purple_conv_present_error(purple_conversation_get_name(conv_p), purple_connection_get_account(gc_p), LURCH_ERR_STRING_ENCRYPT);
    purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
    free(err_msg_dbg);
    *msg_stanza_pp = (void *) 0;
  }
  if (ret_val) {
    omemo_message_destroy(om_msg_p);
    g_list_free_full(addr_l_p, lurch_addr_list_destroy_func);
  }

  free(uname);
  free(db_fn_omemo);
  axc_context_destroy_all(axc_ctx_p);
  free(tempxml);
  omemo_devicelist_destroy(user_dl_p);
}
#endif

#if 0
// not needed in this Omemo class. Just here for reference to lurch
static void lurch_xml_sent_cb(PurpleConnection * gc_p, xmlnode ** stanza_pp) {
  xmlnode * body_node_p      = (void *) 0;
  xmlnode * encrypted_node_p = (void *) 0;
  char * node_name           = (void *) 0;
  const char * type          = (void *) 0;

  if (uninstall) {
    return;
  }

  if (!stanza_pp || !*stanza_pp) {
    return;
  }

  node_name = (*stanza_pp)->name;
  type = xmlnode_get_attrib(*stanza_pp, "type");

  if (!g_strcmp0(node_name, "message")) {
    body_node_p = xmlnode_get_child(*stanza_pp, "body");
    if (!body_node_p) {
      return;
    }

    encrypted_node_p = xmlnode_get_child(*stanza_pp, "encrypted");
    if (encrypted_node_p) {
      return;
    }

    if (!g_strcmp0(type, "chat")) {
      lurch_message_encrypt_im(gc_p, stanza_pp);
    } else if (!g_strcmp0(type, "groupchat")) {
      lurch_message_encrypt_groupchat(gc_p, stanza_pp);
    }
  }
}
#endif

/**
 * Callback for the "receiving xmlnode" signal.
 * Decrypts message, if applicable.
 */
std::string Omemo::messageDecrypt(const std::string& message)
{
    // lurch_message_decrypt
    int ret_val{0};
    char * err_msg_dbg{nullptr};
    //int len;

    omemo_message * msg_p{nullptr};
    //char * uname = nullptr;
    char * db_fn_omemo{nullptr};
    axc_context * axc_ctx_p{nullptr};
    uint32_t own_id{0};
    uint8_t * key_p{nullptr};
    size_t key_len{0};
    axc_buf * key_buf_p{nullptr};
    axc_buf * key_decrypted_p{nullptr};
    char * sender_name{nullptr};
    axc_address sender_addr{};
    char * bundle_node_name{nullptr};
    omemo_message * keytransport_msg_p{nullptr};
    char * xml{nullptr};
    char * sender{nullptr};
    char ** split{nullptr};
    char * room_name{nullptr};
    char * buddy_nick{nullptr};
    //xmlnode * plaintext_msg_node_p = nullptr;
    char * recipient_bare_jid{nullptr};
    char* pMsg{nullptr};
    std::string decryptedMsg{};
    std::string sType{};
    std::string sFrom{};
    std::string sTo{};

    //const char * type = xmlnode_get_attrib(*msg_stanza_pp, "type");
    //const char * from = xmlnode_get_attrib(*msg_stanza_pp, "from");
    //const char * to   = xmlnode_get_attrib(*msg_stanza_pp, "to");
    const char* type{nullptr};
    const char* from{nullptr};
    const char* to{nullptr};

    QString qsType = XmlProcessor::getContentInTag("message", "type", QString::fromStdString(message));
    QString qsFrom = XmlProcessor::getContentInTag("message", "from", QString::fromStdString(message));
    QString qsTo = XmlProcessor::getContentInTag("message", "to", QString::fromStdString(message));

    if ( qsType.isEmpty() || qsFrom.isEmpty())
    {
        qDebug() << "from or type not found in message node!";
        goto cleanup;
    }
    sType.assign(qsType.toStdString());
    sFrom.assign(qsFrom.toStdString());
    sTo.assign(qsTo.toStdString());
    type = sType.c_str();
    from = sFrom.c_str();
    to = sTo.c_str();

    if (uninstall_) {
        goto cleanup;
    }

    //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(gc_p)));
    db_fn_omemo = unameGetDbFn(uname_, (char*)LURCH_DB_NAME_OMEMO);

    // on prosody and possibly other servers, messages to the own account do not have a recipient
    if (!to) {
      recipient_bare_jid = strdup(uname_);
    } else {
      recipient_bare_jid = strdup(Swift::JID(sTo).toBare().toString().c_str());
    }


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

    ret_val = lurch_util_axc_get_init_ctx(uname_, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to get axc ctx for %s", uname_);
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
    sender_addr.device_id = (int32_t)omemo_message_get_sender_id(msg_p);

    ret_val = axc_pre_key_message_process(key_buf_p, &sender_addr, axc_ctx_p, &key_decrypted_p);
    if (ret_val == AXC_ERR_NOT_A_PREKEY_MSG) {
        if (axc_session_exists_initiated(&sender_addr, axc_ctx_p)) {
            ret_val = axc_message_decrypt_from_serialized(key_buf_p, &sender_addr, axc_ctx_p, &key_decrypted_p);
            if (ret_val) {
                if (ret_val == SG_ERR_DUPLICATE_MESSAGE && !g_strcmp0(sender, uname_) && !g_strcmp0(recipient_bare_jid, uname_)) {
                  // in combination with message carbons, sending a message to your own account results in it arriving twice
                  purple_debug_warning("lurch", "ignoring decryption error due to a duplicate message from own account to own account\n");
                  //*msg_stanza_pp = (void *) 0;
                  goto cleanup;
                } else {
                  err_msg_dbg = g_strdup_printf("failed to decrypt key");
                  goto cleanup;
                }
            }
        } else {
            purple_debug_info("lurch", "received omemo message but no session with the device exists, ignoring\n");
            goto cleanup;
        }
    } else if (ret_val == AXC_ERR_INVALID_KEY_ID) {
        ret_val = omemo_bundle_get_pep_node_name((uint32_t)sender_addr.device_id, &bundle_node_name);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to get bundle pep node name");
            goto cleanup;
        }

#if 0
        <iq type='get' from='juliet@capulet.lit' to='romeo@montague.lit' id='gfetch0'>
          <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node="eu.siacs.conversations.axolotl.bundles:123"></items>
          </pubsub>
        </iq>
#endif
#if 0
        jabber_pep_request_item(purple_connection_get_protocol_data(gc_p),
                                sender_addr.name, bundle_node_name,
                                (void *) 0,
                                lurch_pep_bundle_for_keytransport);
#endif
        const std::string bundleRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(bundle_node_name) + "'/></pubsub>";
        RawRequestWithFromJid::ref requestDeviceList = RawRequestWithFromJid::create(Swift::IQ::Get, std::string(sender_addr.name), bundleRequestXml, client_->getIQRouter());
        requestDeviceList->onResponse.connect(boost::bind(&Omemo::pepBundleForKeytransport, this, _1, _2));
        requestDeviceList->send();

        // not in lurch, but needed here imho...
        goto cleanup;

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

    decryptedMsg.assign(xml);
#if 0
    plaintext_msg_node_p = xmlnode_from_str(xml, -1);

    // libpurple doesn't know what to do with incoming messages addressed to someone else, so they need to be written to the conversation manually
    // incoming messages from the own account in MUCs are fine though
    if (!g_strcmp0(sender, uname) && !g_strcmp0(type, "chat")) {
        conv_p = purple_find_conversation_with_account(PURPLE_CONV_TYPE_IM, recipient_bare_jid, purple_connection_get_account(gc_p));
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
    //free(uname);
    free(db_fn_omemo);
    free(recipient_bare_jid);
    omemo_message_destroy(keytransport_msg_p);
    omemo_message_destroy(msg_p);

    return decryptedMsg;
}

void Omemo::handleDeviceListResponse(const Swift::JID jid, const std::string& str)
{
    // implements lurch_pep_devicelist_event_handler

    std::string bareJidStr = jid.toBare().toString();
    QString qJid = QString::fromStdString(bareJidStr);
    qDebug() << "OMEMO: handle device list Response. Jid: " << qJid << ". list: " << QString::fromStdString(str);

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


    // get the items of the request
    QString items = XmlProcessor::getChildFromNode("items", QString::fromStdString(str));

    // call this methods even with empty 'items'! Its an init path to the bundleList.
    if (myBareJid_.compare(qJid, Qt::CaseInsensitive) == 0)
    {
        // was a request for my device list
        ownDeviceListRequestHandler(items);
    }
    else
    {
        // requested someone else device list
        if ( (!items.isEmpty()) && items.startsWith("<items"))
        {
            qDebug() << "requested someone else device list: " << QString::fromStdString(bareJidStr) << ": " << items;

            omemo_devicelist* dl_in_p = nullptr;
            char* pItems = strdup(items.toStdString().c_str());

            int ret = omemo_devicelist_import(pItems, bareJidStr.c_str(), &dl_in_p);
            if ( ret == 0)
            {
                if(devicelistProcess(uname_, dl_in_p) != 0)
                {
                    qDebug() << "failed to process devicelist";
                }
                else
                {
                    emit signalReceivedDeviceListOfJid(qJid);
                }

                omemo_devicelist_destroy(dl_in_p);
            }
            else
            {
                qDebug() << "failed to import devicelist: " << ret;
            }
            free(pItems);
        }
    }
}

void Omemo::publishedDeviceList(const std::string& str)
{
    // FIXME check if there was an error on device list publishing
    qDebug() << "OMEMO: publishedDeviceList: " << QString::fromStdString(str);
}

void Omemo::publishedBundle(const std::string& str)
{
    // FIXME check if there was an error on bundle publishing
    qDebug() << "OMEMO: publishedBundle: " << QString::fromStdString(str);
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

// FIXME not just request the device list in this slot but also subscribe to the pep node!
void Omemo::slotRequestDeviceList(QString humanBareJid)
{
    qDebug() << "request device list for " << humanBareJid;
    requestDeviceList(Swift::JID(humanBareJid.toStdString()));
}

#pragma GCC diagnostic pop
