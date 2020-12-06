#ifndef OMEMO_H
#define OMEMO_H

#include "LurchTypes.h"

#include <QObject>
#include <QMap>
#include <QString>

#include <Swiften/Swiften.h>

extern "C" {
#include "axc.h"
#include "libomemo.h"
#include "libomemo_crypto.h"
}

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = nullptr);
    ~Omemo();
    void setupWithClient(Swift::Client* client);
    //QString encryptMessage(const QString& msg);
    std::string messageEncryptIm(const std::string msg_stanza_pp);
    std::string messageDecrypt(const std::string& message);


signals:

public slots:

private:
    typedef struct lurch_addr {
      char * jid;
      uint32_t device_id;
    } lurch_addr;

    typedef enum {
        JABBER_IQ_SET,
        JABBER_IQ_GET,
        JABBER_IQ_RESULT,
        JABBER_IQ_ERROR,
        JABBER_IQ_NONE
    } JabberIqType;

    typedef struct lurch_queued_msg lurch_queued_msg;

    void requestDeviceList(const Swift::JID& jid);
    void ownDeviceListRequestHandler(QString items);

    void handleDeviceListResponse(const Swift::JID jid, const std::string &str);
    void publishedDeviceList(const std::string& str);
    void publishedBundle(const std::string& str);
    void requestBundleHandler(const Swift::JID &jid, const std::string &bundleId, lurch_queued_msg *qMsg, const std::string& str);

    void pepBundleForKeytransport(const std::string from, const std::string& items);

    std::string msgFinalizeEncryption(axc_context * axc_ctx_p, omemo_message * om_msg_p, GList * addr_l_p, const std::string& msg_stanza_pp);
    int bundleRequestDo(const char * to, uint32_t device_id, lurch_queued_msg * qmsg_p);

    bool axcPrepare(QString fromJid);
    bool axcGetInitCtx(axc_context** ctx_pp);
    int bundlePublishOwn();
    int devicelistProcess(const char *uname, omemo_devicelist * dl_in_p);
    void accountConnectCb();
    void BundleRequestCb(const std::string& fromStr, JabberIqType type, const std::string& idStr,
                                const std::string& packet_p, lurch_queued_msg *qmsg_p);

    char* unameGetDbFn(const char * uname, char * which);
    int bundleCreateSession(const char* from, const std::string& items, axc_context * axc_ctx_p);
    int keyEncrypt(const lurch_addr * recipient_addr_p, const uint8_t * key_p, size_t key_len, axc_context * axc_ctx_p, axc_buf ** key_ct_buf_pp);

    GList* lurch_addr_list_add(GList * addrs_p, const omemo_devicelist * dl_p, const uint32_t * exclude_id_p);
    static void lurch_addr_list_destroy_func(gpointer data);
    int lurch_axc_sessions_exist(GList * addr_l_p, axc_context * axc_ctx_p, GList ** no_sess_l_pp);
    int lurch_msg_encrypt_for_addrs(omemo_message * om_msg_p, GList * addr_l_p, axc_context * axc_ctx_p);
    int lurch_key_encrypt(const lurch_addr * recipient_addr_p, const uint8_t * key_p, size_t key_len, axc_context * axc_ctx_p, axc_buf ** key_ct_buf_pp);
    int lurch_export_encrypted(omemo_message * om_msg_p, char ** xml_pp);
    int lurch_queued_msg_create(omemo_message * om_msg_p, GList * recipient_addr_l_p, GList * no_sess_l_p, lurch_queued_msg ** qmsg_pp);

    void handleConnected();

    bool isEncryptedMessage(const QString& xmlNode);

    Swift::Client* client_{};
    QString deviceListNodeName_{};
    QString myBareJid_{};
    char* uname_{nullptr};

    int uninstall_{0};

    omemo_crypto_provider crypto = {
        .random_bytes_func = omemo_default_crypto_random_bytes,
        .aes_gcm_encrypt_func = omemo_default_crypto_aes_gcm_encrypt,
        .aes_gcm_decrypt_func = omemo_default_crypto_aes_gcm_decrypt
    };
};

#endif // OMEMO_H
