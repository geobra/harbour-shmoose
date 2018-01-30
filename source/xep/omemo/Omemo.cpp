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
#include <QTimer>

#include <QDebug>

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

// key: char * (name of muc), value: GHashTable * (mapping of user alias to full jid)
GHashTable * chat_users_ht_p = nullptr;


Omemo::Omemo(QObject *parent) : QObject(parent),
    client_(NULL), uninstall(0), lastNodeName_(""), lastType_(""), lastFrom_(""), lastId_("")
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

void Omemo::lurch_addr_list_destroy_func(gpointer data) {
  lurch_addr * addr_p = (lurch_addr *) data;
  free(addr_p->jid);
  free(addr_p);
}

void Omemo::cleanupDeviceList()
{
    const std::string deviceListEmptyXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><publish node='eu.siacs.conversations.axolotl.devicelist'><item><list xmlns='eu.siacs.conversations.axolotl'></list></item></publish></pubsub>";
    Swift::RawRequest::ref emptyDeviceList = Swift::RawRequest::create(Swift::IQ::Set, client_->getJID().toBare(), deviceListEmptyXml, client_->getIQRouter());
    emptyDeviceList->send();
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
    char * err_msg_dbg = nullptr;

    lurch_queued_msg * qmsg_p = nullptr;
    GHashTable * sess_handled_p = nullptr;

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
char * Omemo::lurch_uname_get_db_fn(const char * uname, char * which)
{
    std::string omemoPath = System::getOmemoPath().toStdString();
    return g_strconcat(omemoPath.c_str(), "/", uname, "_", which, LURCH_DB_SUFFIX, NULL);
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
    char * err_msg_dbg = nullptr;

    axc_buf * key_buf_p = nullptr;
    axc_buf * key_ct_buf_p = nullptr;
    axc_address axc_addr = {0};

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
    char * err_msg_dbg = nullptr;

    GList * curr_l_p = nullptr;
    lurch_addr * curr_addr_p = nullptr;
    axc_address addr = {0};
    axc_buf * curr_key_ct_buf_p = nullptr;

    purple_debug_info("lurch", "%s: trying to encrypt key for %i devices\n", __func__, g_list_length(addr_l_p));

    for (curr_l_p = addr_l_p; curr_l_p; curr_l_p = curr_l_p->next) {
        curr_addr_p = (lurch_addr *) curr_l_p->data;
        addr.name = curr_addr_p->jid;
        addr.name_len = strnlen(addr.name, JABBER_MAX_LEN_BARE);
        addr.device_id = curr_addr_p->device_id;

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
 * Creates an axc session from a received bundle.
 *
 * @param uname The own username.
 * @param from The sender of the bundle.
 * @param items_p The bundle update as received in the PEP request handler.
 */
int Omemo::lurch_bundle_create_session(const char * uname, const char * from, const char* items_p, axc_context * axc_ctx_p) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    //int len;
    omemo_bundle * om_bundle_p = nullptr;
    axc_address remote_addr = {0};
    uint32_t pre_key_id = 0;
    uint8_t * pre_key_p = nullptr;
    size_t pre_key_len = 0;
    uint32_t signed_pre_key_id = 0;
    uint8_t * signed_pre_key_p = nullptr;
    size_t signed_pre_key_len = 0;
    uint8_t * signature_p = nullptr;
    size_t signature_len = 0;
    uint8_t * identity_key_p = nullptr;
    size_t identity_key_len = 0;
    axc_buf * pre_key_buf_p = nullptr;
    axc_buf * signed_pre_key_buf_p = nullptr;
    axc_buf * signature_buf_p = nullptr;
    axc_buf * identity_key_buf_p = nullptr;

    purple_debug_info("lurch", "%s: creating a session between %s and %s from a received bundle\n", __func__, uname, from);

    ret_val = omemo_bundle_import(items_p, &om_bundle_p);
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

/**
 * Implements JabberIqCallback.
 * Callback for a bundle request.
 */
void Omemo::lurch_bundle_request_cb(const char * from, const char * id, const char * items_p, gpointer data_p) {
    int ret_val = 0;
    char * err_msg_conv = nullptr;
    char * err_msg_dbg = nullptr;

    char ** split = nullptr;
    char * device_id_str = nullptr;
    axc_address addr = {0};
    axc_context * axc_ctx_p = nullptr;
    char * recipient = nullptr;
    //xmlnode * pubsub_node_p = nullptr;
    //xmlnode * items_node_p = nullptr;
    int msg_handled = 0;
    char * addr_key = nullptr;
    char * msg_xml = nullptr;
    //xmlnode * msg_node_p = nullptr;

    // FIXME implement the queue...
    //lurch_queued_msg * qmsg_p = (lurch_queued_msg *) data_p;

    //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(js_p->gc)));
    std::string bareJid = client_->getJID().toBare().toString();
    QByteArray JidArray = QString::fromStdString(bareJid).toLocal8Bit();
    char* uname = JidArray.data();

    // FIXME implement the queue...
    //recipient = omemo_message_get_recipient_name_bare(qmsg_p->om_msg_p);

    if (!from) {
        // own user
        from = uname;
    }

    split = g_strsplit(id, "#", 3);
    device_id_str = split[1];

    purple_debug_info("lurch", "%s: %s received bundle update from %s:%s\n", __func__, uname, from, device_id_str);

    addr.name = from;
    addr.name_len = strnlen(from, JABBER_MAX_LEN_BARE);
    addr.device_id = strtol(device_id_str, nullptr, 10);

    ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
    if (ret_val) {
        err_msg_dbg = "failed to get axc ctx";
        goto cleanup;
    }

    if (QString::compare(lastType_, "error", Qt::CaseInsensitive) == 0) {
    //if (type == JABBER_IQ_ERROR) {
        err_msg_conv = g_strdup_printf("The device %s owned by %s does not have a bundle and will be skipped. "
                                       "The owner should fix this, or remove the device from the list.", device_id_str, from);
    }
    else
    {
        // item_p direct passed in this function
#if 0
        pubsub_node_p = xmlnode_get_child(packet_p, "pubsub");
        if (!pubsub_node_p) {
            ret_val = LURCH_ERR;
            err_msg_dbg = "no <pubsub> node in response";
            goto cleanup;
        }

        items_node_p = xmlnode_get_child(pubsub_node_p, "items");
        if (!items_node_p) {
            ret_val = LURCH_ERR;
            err_msg_dbg = "no <items> node in response";
            goto cleanup;
        }
#endif
        ret_val = axc_session_exists_initiated(&addr, axc_ctx_p);
        if (!ret_val) {
            ret_val = lurch_bundle_create_session(uname, from, items_p, axc_ctx_p);
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

    // FIXME implement the queue...
#if 0
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

        ret_val = omemo_message_export_encrypted(qmsg_p->om_msg_p, OMEMO_ADD_MSG_EME, &msg_xml);
        if (ret_val) {
            err_msg_dbg = "failed to export the message to xml";
            goto cleanup;
        }

        qDebug() << "### FIXME sending enc msg: " << msg_xml;

#if 0
        msg_node_p = xmlnode_from_str(msg_xml, -1);
        if (!msg_node_p) {
            err_msg_dbg = "failed to parse xml from string";
            ret_val = LURCH_ERR;
            goto cleanup;
        }

        purple_debug_info("lurch", "sending encrypted msg\n");
        purple_signal_emit(purple_plugins_find_with_id("prpl-jabber"), "jabber-sending-xmlnode", js_p->gc, &msg_node_p);
#endif

        lurch_queued_msg_destroy(qmsg_p);
    }
#endif

cleanup:
    if (err_msg_conv) {
        // FIXME display error to user
        //purple_conv_present_error(recipient, purple_connection_get_account(js_p->gc), err_msg_conv);
        qDebug() << err_msg_conv;
        g_free(err_msg_conv);
    }
    if (err_msg_dbg) {
        // FIXME display error to user
        //purple_conv_present_error(recipient, purple_connection_get_account(js_p->gc), LURCH_ERR_STRING_ENCRYPT);
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
    }

    //free(uname);
    g_strfreev(split);
    axc_context_destroy_all(axc_ctx_p);
    free(addr_key);
    free(recipient);
    free(msg_xml);
    //if (msg_node_p) {
    //    xmlnode_free(msg_node_p);
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
int Omemo::lurch_bundle_request_do(
		const char * to, 
		uint32_t device_id, 
		lurch_queued_msg * qmsg_p) {

    /*
     * <iq type='get' from='romeo@montague.lit' to='juliet@capulet.lit' id='fetch1'>
          <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node='eu.siacs.conversations.axolotl.bundles:31415 max_items='1'/>
          </pubsub>
       </iq>
     */

#if 0
    int ret_val = 0;

    JabberIq * jiq_p = nullptr;
    xmlnode * pubsub_node_p = nullptr;
    char * device_id_str = nullptr;
    char * rand_str = nullptr;
    char * req_id = nullptr;
    char * bundle_node_name = nullptr;
    xmlnode * items_node_p = nullptr;

    purple_debug_info("lurch", "%s: %s is requesting bundle from %s:%i\n", __func__,
                      purple_account_get_username(purple_connection_get_account(js_p->gc)), to, device_id);

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

    // FIXME how to connect the pointer to the waiting message with the connected method of incoming bundles
    requestBundle(device_id, Swift::JID(to));
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

         //char * strippedXmlCStr = "<items node=\"eu.siacs.conversations.axolotl.devicelist\"><item id=\"COFFEEBABE\"><list xmlns=\"eu.siacs.conversations.axolotl\"><device id=\"1234567890\"></device></list></item></items>";


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
    install = 0; // FIXME set to 0 if already installed

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

    if (items_p.empty()) { // FIXME check if this function will be called on non existing pep device list
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
        QString strippedXml = getChildFromNode("items", QString::fromStdString(items_p));

        QByteArray xmlArray = strippedXml.toLocal8Bit();
        char* strippedXmlCStr = xmlArray.data();

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
 * A JabberPEPHandler function.
 * On receiving a devicelist PEP update it updated the database.
 */
void Omemo::lurch_pep_devicelist_event_handler(const char * from, const std::string &items_p) {
    int ret_val = 0;
    //int len = 0;
    char * err_msg_dbg = nullptr;
    char* tmp_items_p = nullptr;

    //char * uname = (void *) 0;
    std::string bareJid = client_->getJID().toBare().toString();
    QByteArray JidArray = QString::fromStdString(bareJid).toLocal8Bit();
    char* uname = JidArray.data();

    omemo_devicelist * dl_in_p = nullptr;

    if (!strncmp(uname, from, strnlen(uname, JABBER_MAX_LEN_BARE))) {
        //own devicelist is dealt with in own handler
        lurch_pep_own_devicelist_request_handler(items_p);
        goto cleanup;
    }

    purple_debug_info("lurch", "%s: %s received devicelist update from %s\n", __func__, uname, from);

    tmp_items_p = strdup(items_p.c_str());
    ret_val = omemo_devicelist_import(tmp_items_p, from, &dl_in_p);
    free(tmp_items_p);

    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to import devicelist");
        goto cleanup;
    }

    ret_val = lurch_devicelist_process(uname, dl_in_p);
    if(ret_val) {
        err_msg_dbg = g_strdup_printf("failed to process devicelist");
        goto cleanup;
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        g_free(err_msg_dbg);
    }
    //g_free(uname);
    omemo_devicelist_destroy(dl_in_p);
}

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

    GList * no_sess_l_p = nullptr;

    GList * curr_p;
    lurch_addr * curr_addr_p;
    axc_address curr_axc_addr = {0};
    for (curr_p = addr_l_p; curr_p; curr_p = curr_p->next) {
        curr_addr_p = (lurch_addr *) curr_p->data;

        curr_axc_addr.name = curr_addr_p->jid;
        curr_axc_addr.name_len = strnlen(curr_axc_addr.name, JABBER_MAX_LEN_BARE);
        curr_axc_addr.device_id = curr_addr_p->device_id;

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
    GList * dl_l_p = nullptr;
    GList * curr_p = nullptr;
    lurch_addr curr_addr = {0};
    uint32_t curr_id = 0;
    lurch_addr * temp_addr_p = nullptr;

    curr_addr.jid = g_strdup(omemo_devicelist_get_owner(dl_p));

    dl_l_p = omemo_devicelist_get_id_list(dl_p);

    for (curr_p = dl_l_p; curr_p; curr_p = curr_p->next) {
        curr_id = omemo_devicelist_list_data(curr_p);
        if (exclude_id_p && *exclude_id_p == curr_id) {
            continue;
        }

        curr_addr.device_id = curr_id;

        temp_addr_p = (lurch_addr *)malloc(sizeof(lurch_addr));
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
/*JabberStream * js_p,*/
int Omemo::lurch_msg_finalize_encryption(axc_context * axc_ctx_p, omemo_message * om_msg_p, GList * addr_l_p, char ** msg_stanza_pp) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;

    GList * no_sess_l_p = nullptr;
    char * xml = nullptr;
    //xmlnode * temp_node_p = nullptr;
    lurch_queued_msg * qmsg_p = nullptr;
    GList * curr_item_p = nullptr;
    lurch_addr curr_addr = {0};
    char * bundle_node_name = nullptr;

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

        ret_val = omemo_message_export_encrypted(om_msg_p, OMEMO_ADD_MSG_EME, &xml);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to export omemo msg to xml");
            goto cleanup;
        }

        //temp_node_p = xmlnode_from_str(xml, -1);
        //*msg_stanza_pp = temp_node_p;
        *msg_stanza_pp = xml;

        qDebug() << "#######ENC#####" << xml;

        QString to = getValueForElementInNode("message", QString(xml), "to");
        QString encryptedPayload = getChildFromNode("encrypted", QString(xml));
        sendEncryptedMessage(to, encryptedPayload);

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

            ret_val = omemo_bundle_get_pep_node_name(curr_addr.device_id, &bundle_node_name);
            if (ret_val) {
                err_msg_dbg = g_strdup_printf("failed to get pep node name");
                goto cleanup;
            }

            lurch_bundle_request_do(curr_addr.jid,
                                    curr_addr.device_id,
                                    qmsg_p);

        }
        //*msg_stanza_pp = (void *) 0;
    }

cleanup:
    if (err_msg_dbg) {
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
        *msg_stanza_pp = nullptr;
    }
    if (!qmsg_p || ret_val) {
        free(qmsg_p);
    }

    free(xml);
    free(bundle_node_name);

    return ret_val;
}

// FIXME move to MessageHandler
void Omemo::sendEncryptedMessage(const QString &to, const QString &encryptedPayload)
{
    Swift::Message::ref msg(new Swift::Message);
    Swift::JID receiverJid(to.toStdString());

    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();

    msg->setID(msgId);
    msg->setFrom(Swift::JID(client_->getJID()));
    msg->setTo(receiverJid);

    Swift::RawXMLPayload::ref encPayload(new Swift::RawXMLPayload(encryptedPayload.toStdString() + "<encryption xmlns=\"urn:xmpp:eme:0\" namespace=\"eu.siacs.conversations.axolotl\" name=\"OMEMO\" /><store xmlns=\"urn:xmpp:hints\" />"));
    msg->addPayload(encPayload);

    client_->sendMessage(msg);
}

/**
 * Set as callback for the "sending xmlnode" signal.
 * Encrypts the message body, if applicable.
 */
/*PurpleConnection * gc_p, xmlnode ** msg_stanza_pp*/
// FIXME pass in complete message xml from MessageHandler before sending it!
void Omemo::lurch_message_encrypt_im(const QString& receiver, const QString& message) {
    int ret_val = 0;
    char * err_msg_dbg = nullptr;
    //int len = 0;

    //PurpleAccount * acc_p = nullptr;
    char * uname = nullptr;
    char * db_fn_omemo = nullptr;
    const char * to = nullptr;
    omemo_devicelist * dl_p = nullptr;
    GList * recipient_dl_p = nullptr;
    omemo_devicelist * user_dl_p = nullptr;
    GList * own_dl_p = nullptr;
    axc_context * axc_ctx_p = nullptr;
    uint32_t own_id = 0;
    omemo_message * msg_p = nullptr;
    GList * addr_l_p = nullptr;
    char * recipient = nullptr;
    char * tempxml = nullptr;
    char* xml_node_msg = nullptr;

    //FIXME
    QString msgToSend = "<message to='" + receiver + "' id='asdfasdf324fd3456'><body>" + message + "</body></message>";
    QByteArray msgToSendArray = msgToSend.toLocal8Bit();

    //recipient = jabber_get_bare_jid(xmlnode_get_attrib(*msg_stanza_pp, "to"));
    QByteArray recipientBuffer = receiver.toLocal8Bit();
    recipient = recipientBuffer.data();

    //acc_p = purple_connection_get_account(gc_p);
    //uname = lurch_uname_strip(purple_account_get_username(acc_p));
    std::string bareJid = client_->getJID().toBare().toString();
    QByteArray JidArray = QString::fromStdString(bareJid).toLocal8Bit();
    uname = JidArray.data();

    db_fn_omemo = lurch_uname_get_db_fn(uname, LURCH_DB_NAME_OMEMO);

    ret_val = omemo_storage_chatlist_exists(recipient, db_fn_omemo);
    if (ret_val < 0) {
        err_msg_dbg = g_strdup_printf("failed to look up %s in DB %s", recipient, db_fn_omemo);
        goto cleanup;
    } else if (ret_val == 1) {
        purple_debug_info("lurch", "%s: %s is on blacklist, skipping encryption\n", __func__, recipient);
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
    //tempxml = xmlnode_to_str(*msg_stanza_pp, &len);

    tempxml = msgToSendArray.data();

    ret_val = omemo_message_prepare_encryption(tempxml, own_id, &crypto_, OMEMO_STRIP_ALL, &msg_p);
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

    //free(tempxml);

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
            err_msg_dbg = g_strdup_printf("failed to check if session exists for %s in %s's db\n", to, uname);
            goto cleanup;
        } else if (ret_val == 1) {
            //purple_conv_present_error(recipient, purple_connection_get_account(gc_p), "Even though an encrypted session exists, the recipient's devicelist is empty."
            //                                                                          "The user probably uninstalled OMEMO, so you can add this conversation to the blacklist.");
            // FIXME
            std::cout << "Even though an encrypted session exists, the recipient's devicelist is empty.";
        } else {
            goto cleanup;
        }
    }

    ret_val = omemo_storage_user_devicelist_retrieve(uname, db_fn_omemo, &user_dl_p);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to retrieve devicelist for %s", uname);
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
    addr_l_p = lurch_addr_list_add(addr_l_p, dl_p, nullptr);
    if (!addr_l_p) {
        err_msg_dbg = g_strdup_printf("failed to malloc address struct");
        ret_val = LURCH_ERR_NOMEM;
        goto cleanup;
    }

    ret_val = lurch_msg_finalize_encryption(axc_ctx_p, msg_p, addr_l_p, &xml_node_msg);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to finalize omemo message");
        goto cleanup;
    }

cleanup:
    if (err_msg_dbg) {
        // FIXME
        //purple_conv_present_error(recipient, purple_connection_get_account(gc_p), LURCH_ERR_STRING_ENCRYPT);
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
        //*msg_stanza_pp = (void *) 0;
    }
    if (ret_val) {
        omemo_message_destroy(msg_p);
        g_list_free_full(addr_l_p, lurch_addr_list_destroy_func);
    }
    //free(recipient);
    //free(uname);
    free(db_fn_omemo);
    omemo_devicelist_destroy(dl_p);
    g_list_free_full(recipient_dl_p, free);
    omemo_devicelist_destroy(user_dl_p);
    g_list_free_full(own_dl_p, free);
    axc_context_destroy_all(axc_ctx_p);
    //free(tempxml);
}



/**
 * Callback for the "receiving xmlnode" signal.
 * Decrypts message, if applicable.
 */
void Omemo::lurch_message_decrypt(const char* from, const char* type, std::string msg_stanza_pp) {
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
    axc_address sender_addr = {0};
    char * bundle_node_name = nullptr;
    omemo_message * keytransport_msg_p = nullptr;
    char * xml = nullptr;
    char * sender = nullptr;
    char ** split = nullptr;
    char * room_name = nullptr;
    char * buddy_nick = nullptr;
    GHashTable * nick_jid_ht_p = nullptr;
    //xmlnode * plaintext_msg_node_p = nullptr;
    char * recipient_bare_jid = nullptr;
    //PurpleConversation * conv_p = nullptr;
    char* tmp_msg_p = nullptr;

//    const char * type = xmlnode_get_attrib(*msg_stanza_pp, "type");
//    const char * from = xmlnode_get_attrib(*msg_stanza_pp, "from");

    std::string bareJid = client_->getJID().toBare().toString();
    QByteArray JidArray = QString::fromStdString(bareJid).toLocal8Bit();

    if (uninstall) {
        goto cleanup;
    }

    //uname = lurch_uname_strip(purple_account_get_username(purple_connection_get_account(gc_p)));
    uname = JidArray.data();

    db_fn_omemo = lurch_uname_get_db_fn(uname, LURCH_DB_NAME_OMEMO);

    if (!g_strcmp0(type, "chat")) {

        //sender = jabber_get_bare_jid(from);
        QByteArray qSender = QString::fromStdString((Swift::JID(from).toBare().toString())).toLocal8Bit();
        sender = qSender.data();

        ret_val = omemo_storage_chatlist_exists(sender, db_fn_omemo);
        if (ret_val < 0) {
            err_msg_dbg = g_strdup_printf("failed to look up %s in %s", sender, db_fn_omemo);
            goto cleanup;
        } else if (ret_val == 1) {
            // FIXME
            //purple_conv_present_error(sender, purple_connection_get_account(gc_p), "Received encrypted message in blacklisted conversation.");
            qDebug() << "Received encrypted message in blacklisted conversation.";
        }
    } else if (!g_strcmp0(type, "groupchat")) {
        // FIXME groupchat encryption
        qDebug() << "FIXME groupchat encryption";
#if 0
        split = g_strsplit(from, "/", 2);
        room_name = split[0];
        buddy_nick = split[1];

        ret_val = omemo_storage_chatlist_exists(room_name, db_fn_omemo);
        if (ret_val < 0) {
            err_msg_dbg = g_strdup_printf("failed to look up %s in %s", room_name, db_fn_omemo);
            goto cleanup;
        } else if (ret_val == 0) {
            // FIXME
            //purple_conv_present_error(room_name, purple_connection_get_account(gc_p), "Received encrypted message in non-OMEMO room.");
            qDebug() << "eceived encrypted message in non-OMEMO room.";
        }

        nick_jid_ht_p = g_hash_table_lookup(chat_users_ht_p, room_name);
        sender = g_strdup(g_hash_table_lookup(nick_jid_ht_p, buddy_nick));
#endif
    }

    tmp_msg_p = strdup(msg_stanza_pp.c_str());
    ret_val = omemo_message_prepare_decryption(tmp_msg_p, &msg_p);
    free(tmp_msg_p);

    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed import msg for decryption");
        goto cleanup;
    }

    ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
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
            purple_debug_info("lurch", "received omemo message that does not contain a key for this device, ignoring\n");
            goto cleanup;
        }
    } else if (ret_val == AXC_ERR_INVALID_KEY_ID) {
        ret_val = omemo_bundle_get_pep_node_name(sender_addr.device_id, &bundle_node_name);
        if (ret_val) {
            err_msg_dbg = g_strdup_printf("failed to get bundle pep node name");
            goto cleanup;
        }

        // FIXME implement request of keys!
#if 0
        jabber_pep_request_item(purple_connection_get_protocol_data(gc_p),
                                sender_addr.name, bundle_node_name,
                                (void *) 0,
                                lurch_pep_bundle_for_keytransport);
#endif

    } else if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to prekey msg");
        goto cleanup;
    } else {
        lurch_bundle_publish_own(/*purple_connection_get_protocol_data(gc_p)*/);
    }

    if (!omemo_message_has_payload(msg_p)) {
        purple_debug_info("lurch", "received keytransportmsg\n");
        goto cleanup;
    }

    ret_val = omemo_message_export_decrypted(msg_p, axc_buf_get_data(key_decrypted_p), axc_buf_get_len(key_decrypted_p), &crypto_, &xml);
    if (ret_val) {
        err_msg_dbg = g_strdup_printf("failed to decrypt payload");
        goto cleanup;
    }

    // FIXME do something with encrypted msg node in xml;
    //plaintext_msg_node_p = xmlnode_from_str(xml, -1);
    qDebug() << "##### dec: " << xml;

    if (g_strcmp0(sender, uname)) {
        // FIXME return plaintext msg
        //*msg_stanza_pp = plaintext_msg_node_p;
    } else {
        qDebug() << "FIXME chat not implemented yet!";
#if 0
        if (!g_strcmp0(type, "chat")) {
            recipient_bare_jid = jabber_get_bare_jid(xmlnode_get_attrib(*msg_stanza_pp, "to"));
            conv_p = purple_find_conversation_with_account(PURPLE_CONV_TYPE_CHAT, sender, purple_connection_get_account(gc_p));
            if (!conv_p) {
                conv_p = purple_conversation_new(PURPLE_CONV_TYPE_IM, purple_connection_get_account(gc_p), recipient_bare_jid);;
            }
            purple_conversation_write(conv_p, uname, xmlnode_get_data(xmlnode_get_child(plaintext_msg_node_p, "body")), PURPLE_MESSAGE_SEND, time((void *) 0));
            *msg_stanza_pp = (void *) 0;
        }
#endif
    }

cleanup:
    if (err_msg_dbg) {
        // FIXME error to user
        //purple_conv_present_error(sender, purple_connection_get_account(gc_p), LURCH_ERR_STRING_DECRYPT);
        purple_debug_error("lurch", "%s: %s (%i)\n", __func__, err_msg_dbg, ret_val);
        free(err_msg_dbg);
    }

    g_strfreev(split);
    //free(sender); FIXME
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


void Omemo::setupWithClient(Swift::Client *client)
{
    if (client != NULL)
    {
        client_ = client;

        // FIXME connect msg send, msg receive, pep device list update
        client_->onMessageReceived.connect(boost::bind(&Omemo::handleMessageReceived, this, _1));
        client_->onDataRead.connect(boost::bind(&Omemo::handleDataReceived, this, _1));

        // FIXME for device list updates
        // lurch_pep_devicelist_event_handler

        // FIXME catches to be sent messages and encrypts
        //lurch_xml_sent_cb => lurch_message_encrypt_im || lurch_message_encrypt_groupchat

        // FIXME handles presence and message decryption
        // lurch_xml_received_cb

        // request own device list
        // similar to lurch_account_connect_cb()
        requestDeviceList(client_->getJID().toBare());

        QTimer::singleShot(3000, this, SLOT(shotAfterDelay()));
    }
}

void Omemo::shotAfterDelay()
{
    //cleanupDeviceList();
    //qDebug() << "requst device list for sjde ";
    //requestDeviceList(Swift::JID("x@y.com"));
    //qDebug() << "try to send enc ";
    //lurch_message_encrypt_im("x@y.com", "enc omemo msg1");
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
    qDebug() << "OMEMO: handle device list Response: " << QString::fromStdString(str);

    if (QString::compare(lastType_, "result", Qt::CaseInsensitive) == 0) // else it may be an error.
    {
        // handle own device list
        std::string bareJid = Swift::JID(lastFrom_.toStdString()).toBare().toString();
        std::string myJid = client_->getJID().toBare().toString();

        if (QString::compare(QString::fromStdString(bareJid), QString::fromStdString(myJid), Qt::CaseInsensitive) == 0)
        {
            qDebug() << "handle own device list!";
            lurch_pep_own_devicelist_request_handler(str);
        }
        else
        {
            QString strippedXml = getChildFromNode("items", QString::fromStdString(str));
            QByteArray xmlArray = strippedXml.toLocal8Bit();
            char* strippedXmlCStr = xmlArray.data();

            qDebug() << "new device list " << strippedXml << " for sjd!";
            if (QString::compare(lastNodeName_, "iq", Qt::CaseInsensitive) == 0)
            {
                std::string lastFrom = lastFrom_.toStdString();
                qDebug() << "new device list: handle for " << lastFrom_;
                lurch_pep_devicelist_event_handler(lastFrom.c_str(), strippedXmlCStr);
            }
        }
    }
}




void Omemo::requestBundle(unsigned int device_id, const Swift::JID& jid)
{
    /*
     * <iq type='get' from='romeo@montague.lit' to='juliet@capulet.lit' id='fetch1'>
          <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node='eu.siacs.conversations.axolotl.bundles:31415 max_items='1'/>
          </pubsub>
       </iq>
     */

    char * bundle_node_name = nullptr;

    int ret_val = omemo_bundle_get_pep_node_name(device_id, &bundle_node_name);
    if (ret_val)
    {
        qDebug() << "failed to get bundle pep node name for" << QString::fromStdString(jid.toString());
    }
    else
    {
        // gen the payload
        const std::string bundleRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(bundle_node_name) + "' max_items='1' /></pubsub>";

        qDebug() << "request bundle! " << QString::fromStdString(bundleRequestXml);

        Swift::RawRequest::ref requestBundle = Swift::RawRequest::create(Swift::IQ::Get, jid.toBare(), bundleRequestXml, client_->getIQRouter());
        requestBundle->onResponse.connect(boost::bind(&Omemo::handleBundleResponse, this, _1));
        requestBundle->send();

        g_free(bundle_node_name);
    }
}

void Omemo::handleBundleResponse(const std::string& str)
{
    //qDebug() << "########## OMEMO: handle bundle Response: " << QString::fromStdString(str);

    /*
     *<iq to='x@y.com/shmoose' from='y@x.com' type='result' id='asd'>
     * <pubsub xmlns='http://jabber.org/protocol/pubsub'>
     *  <items node='eu.siacs.conversations.axolotl.bundles:12345'>
     *   <item id='1'><bundle xmlns='eu.siacs.conversations.axolotl'>
     *    <signedPreKeyPublic signedPreKeyId='1'>sfasdfsafd</signedPreKeyPublic>
     *     <signedPreKeySignature>dgdgdsgfdfg</signedPreKeySignature>
     *      <identityKey>dgdsfgdfg</identityKey>
     *       <prekeys>
     * ...
     * </preKeyPublic></prekeys></bundle></item></items></pubsub></iq>"
     */

    QString items = getChildFromNode("items", QString::fromStdString(str));
    std::string stdItems = items.toStdString();

    if (QString::compare(lastNodeName_, "iq", Qt::CaseInsensitive) == 0)
    {
        std::string lastFrom = lastFrom_.toStdString();

        QString deviceIdNode = getValueForElementInNode("items", QString::fromStdString(str), "node");
        if (deviceIdNode.contains(':'))
        {
            std::string deviceId = "foo#" + deviceIdNode.split(':').at(1).toStdString() + "#bar";

            // FIXME dont pass NULL but pointer to qlist!!!
            lurch_bundle_request_cb(lastFrom.c_str(), deviceId.c_str(), stdItems.c_str(), NULL) ;
        }
        else
        {
            qDebug() << "no deviceId in node: " << deviceIdNode;
        }
    }
}

QString Omemo::getFirstNodeNameOfXml(const QString& xml)
{
    QString nodeName = "";

    QDomDocument d;
    if (d.setContent(xml))
    {
        nodeName = d.firstChild().nodeName();
    }

    return nodeName;
}

/*
 * searches for given childElement within xml and returns xml as new root node.
 */
QString Omemo::getChildFromNode(const QString& childElement, const QString &xml)
{
    QString returnXml = "";

    QDomDocument d;
    d.setContent(xml);

    QDomElement element = d.documentElement();
    QDomNode n = element.firstChildElement(childElement);
    if (! n.isNull())
    {
        QDomDocument dd("");
        dd.appendChild(n);
        returnXml = dd.toString(-1);
    }

    return returnXml;
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

QString Omemo::getValueForElementInNode(const QString& node, const QString& xmlNode, const QString& elementString)
{
    QString returnValue = "";

    QDomDocument d;
    if (d.setContent(xmlNode) == true)
    {
        QDomNodeList nodeList1 = d.elementsByTagName(node);
        if (!nodeList1.isEmpty())
        {
            QDomElement element = nodeList1.at(0).toElement();
            QDomNode toNode = element.attributeNode(elementString);

            returnValue = toNode.nodeValue();
        }
    }

    return returnValue;
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

void Omemo::handleDataReceived(Swift::SafeByteArray data)
{
    std::string nodeData = Swift::safeByteArrayToString(data);
    QString qData = QString::fromStdString(nodeData);

    //qDebug() << "####### received: " << qData;
    lastNodeName_ = getFirstNodeNameOfXml(qData);
    if (QString::compare(lastNodeName_, "iq", Qt::CaseInsensitive) == 0 || QString::compare(lastNodeName_, "message", Qt::CaseInsensitive) == 0)
    {
        lastFrom_ = getValueForElementInNode(lastNodeName_, qData, "from");
        lastId_ = getValueForElementInNode(lastNodeName_, qData, "id");
        lastType_ = getValueForElementInNode(lastNodeName_, qData, "type");

        qDebug() << "node data: " << lastNodeName_ << ", " << lastFrom_ << ", " << lastId_;
    }

    if (isEncryptedMessage(qData))
    {
        std::string from = getValueForElementInNode("message", qData, "from").toStdString();
        std::string type = getValueForElementInNode("message", qData, "type").toStdString();

        if ( (! from.empty()) && (! type.empty()) )
        {
            lurch_message_decrypt(from.c_str(), type.c_str(), nodeData);
        }
    }
}

void Omemo::handleMessageReceived(Swift::Message::ref message)
{
    std::cout << "OMEMO: handleMessageReceived: jid: " << message->getFrom() << ", bare: " << message->getFrom().toBare().toString() << ", resource: " << message->getFrom().getResource() << std::endl;
}
