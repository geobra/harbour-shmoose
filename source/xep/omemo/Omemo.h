#pragma once

#include <QObject>
#include <QString>

#include <Swiften/Swiften.h>

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = nullptr);
    ~Omemo();
    void setupWithClient(Swift::Client* client);
    std::string messageEncryptIm(const std::string msg);
    std::string messageDecrypt(const std::string& message);

    void sendAsPepStanza(char* stz);
    void sendRawMessageStanza(char* stz);
    void sendBundleRequest(char* node, void *q_msg);
    void createAndSendBundleRequest(char* sender, char* bundle);

signals:
    void rawMessageStanzaForSending(QString);
    void signalReceivedDeviceListOfJid(QString);

public slots:
    void slotRequestDeviceList(QString humanBareJid);

private:
    void requestDeviceList(const Swift::JID& jid);
    void handleDeviceListResponse(const Swift::JID jid, const std::string &str);
    void publishedDeviceList(const std::string& str);
    void publishedBundle(const std::string& str);
    void requestBundleHandler(const Swift::JID &jid, const std::string &bundleId, void *qMsg, const std::string& str);
    void pepBundleForKeytransport(const std::string from, const std::string& items);
    bool isEncryptedMessage(const QString& xmlNode);

    Swift::Client* client_{};
    QString deviceListNodeName_{};
    QString myBareJid_{};
    char* uname_{nullptr};

    int uninstall_{0};
};
