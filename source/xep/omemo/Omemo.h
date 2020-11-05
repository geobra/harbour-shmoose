#ifndef OMEMO_H
#define OMEMO_H

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

signals:

public slots:

private:
    typedef struct lurch_addr {
      char * jid;
      uint32_t device_id;
    } lurch_addr;

    void requestDeviceList(const Swift::JID& jid);
    void ownDeviceListRequestHandler(QString items);

    void handleDeviceListResponse(const std::string& str);
    void publishedDeviceList(const std::string& str);
    void publishedBundle(const std::string& str);

    void pepBundleForKeytransport(const std::string from, const std::string& items);

    void messageDecrypt(const std::string& message);

    bool axcPrepare(QString fromJid);
    bool axcGetInitCtx(axc_context** ctx_pp);
    int bundlePublishOwn();
    int devicelistProcess(const char *uname, omemo_devicelist * dl_in_p);
    void accountConnectCb();

    char* unameGetDbFn(const char * uname, char * which);
    int bundleCreateSession(const char* from, const std::string& items, axc_context * axc_ctx_p);
    int keyEncrypt(const lurch_addr * recipient_addr_p, const uint8_t * key_p, size_t key_len, axc_context * axc_ctx_p, axc_buf ** key_ct_buf_pp);

    void handleDataReceived(Swift::SafeByteArray data);
    void handleConnected();

    bool isEncryptedMessage(const QString& xmlNode);

    Swift::Client* client_{};
    QString deviceListNodeName_{};
    QString currentNode_{};
    QString myBareJid_{};
    char* uname_{nullptr};

    int uninstall_{0};

    QMap<QString, QString> requestedDeviceListJidIdMap_{};

    omemo_crypto_provider crypto = {
        .random_bytes_func = omemo_default_crypto_random_bytes,
        .aes_gcm_encrypt_func = omemo_default_crypto_aes_gcm_encrypt,
        .aes_gcm_decrypt_func = omemo_default_crypto_aes_gcm_decrypt
    };
};

#endif // OMEMO_H
