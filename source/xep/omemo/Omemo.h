#ifndef OMEMO_H
#define OMEMO_H

extern "C" {
#include "libomemo.h"
#include "libomemo_crypto.h"
#include "libomemo_storage.h"

#include "axc.h"
#include "axc_store.h"
}

#include <QObject>
#include <Swiften/Swiften.h>

#include <QString>
#include <QMap>

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = 0);
    ~Omemo();

    void setupWithClient(Swift::Client *client);

signals:

public slots:
    void shotAfterDelay();

private:

    typedef struct lurch_addr {
      char * jid;
      uint32_t device_id;
    } lurch_addr;

    typedef struct lurch_queued_msg {
      omemo_message * om_msg_p;
      GList * recipient_addr_l_p;
      GList * no_sess_l_p;
      GHashTable * sess_handled_p;
    } lurch_queued_msg;

    void cleanupDeviceList();
    void requestDeviceList(const Swift::JID &jid);
    void handleDeviceListResponse(const std::string &str);

    void requestBundle(unsigned int device_id, const Swift::JID& jid);
    void handleBundleResponse(const std::string& str);

    void publishPepRawXml(const Swift::JID& jid, const QString& rawXml);

    void handleMessageReceived(Swift::Message::ref message);
    void handleDataReceived(Swift::SafeByteArray data);

    bool isEncryptedMessage(const QString& xmlNode);
    void sendEncryptedMessage(const QString& to, const QString &encryptedPayload);

    QString getFirstNodeNameOfXml(const QString &xml);
    QString getChildFromNode(const QString &childElement, const QString &xml);
    QString getValueForElementInNode(const QString& node, const QString& xmlNode, const QString& elementString);

    int lurch_axc_prepare(char * uname);
    int lurch_axc_get_init_ctx(char * uname, axc_context ** ctx_pp);
    char* lurch_uname_get_db_fn(const char * uname, char * which);
    void lurch_pep_own_devicelist_request_handler(const std::string &items_p);
    void lurch_pep_devicelist_event_handler(const char * from, const std::string &items_p);
    int lurch_bundle_publish_own();
    int lurch_devicelist_process(char * uname, omemo_devicelist * dl_in_p /*, JabberStream * js_p*/);

    int lurch_bundle_create_session(const char * uname, const char * from, const char* items_p, axc_context * axc_ctx_p);
    void lurch_bundle_request_cb(const char * from, const char * id, const char * items_p, gpointer data_p);

    char * lurch_queue_make_key_string_s(const char * name, const char * device_id);
    int lurch_queued_msg_is_handled(const lurch_queued_msg * qmsg_p);
    void lurch_queued_msg_destroy(lurch_queued_msg * qmsg_p);

    void lurch_message_encrypt_im(const QString& receiver, const QString &message);
    GList* lurch_addr_list_add(GList * addrs_p, const omemo_devicelist * dl_p, const uint32_t * exclude_id_p);
    int lurch_msg_finalize_encryption(axc_context * axc_ctx_p, omemo_message * om_msg_p, GList * addr_l_p, char **msg_stanza_pp);
    static void lurch_addr_list_destroy_func(gpointer data);
    int lurch_axc_sessions_exist(GList * addr_l_p, axc_context * axc_ctx_p, GList ** no_sess_l_pp);
    int lurch_msg_encrypt_for_addrs(omemo_message * om_msg_p, GList * addr_l_p, axc_context * axc_ctx_p);
    int lurch_queued_msg_create(omemo_message * om_msg_p, GList * recipient_addr_l_p, GList * no_sess_l_p, lurch_queued_msg ** qmsg_pp);
    int lurch_bundle_request_do(const char * to, uint32_t device_id, lurch_queued_msg * qmsg_p);

    int lurch_key_encrypt(const lurch_addr * recipient_addr_p, const uint8_t * key_p, size_t key_len, axc_context * axc_ctx_p, axc_buf ** key_ct_buf_pp);

    void lurch_message_decrypt(const char* from, const char* type, std::string msg_stanza_pp);

    void purple_debug_error (const char *category, const char *format,...);
    void purple_debug_info(const char *category, const char *format,...);

    Swift::Client* client_;

    omemo_crypto_provider crypto_ = {
        .random_bytes_func = omemo_default_crypto_random_bytes,
        .aes_gcm_encrypt_func = omemo_default_crypto_aes_gcm_encrypt,
        .aes_gcm_decrypt_func = omemo_default_crypto_aes_gcm_decrypt
    };

    QString lastNodeName_;
    QString lastType_;
    QString lastFrom_;
    QString lastId_;

    QMap<QString, lurch_queued_msg*> qeuedMessages_;

    int uninstall;
};

#endif // OMEMO_H
