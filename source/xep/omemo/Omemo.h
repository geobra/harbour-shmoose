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
    bool exchangePlainBodyByOmemoStanzas(Swift::Message::ref msg);
    int decryptMessageIfEncrypted(Swift::Message::ref aMessage);

    void sendAsPepStanza(char* stz);
    void sendRawMessageStanza(char* stz);
    void sendBundleRequest(char* node, void *q_msg);
    void createAndSendBundleRequest(char* sender, char* bundle);

    bool isOmemoUser(const QString& bareJid);

signals:
    void rawMessageStanzaForSending(QString);
    void signalReceivedDeviceListOfJid(QString);

public slots:
    void slotInitialRequestDeviceList(QString humanBareJid);

private:
    void determineNamespace(const QString& nsDl);
    std::string messageDecrypt(const std::string& message);
    void requestDeviceList(const Swift::JID& jid);
    void handleDeviceListResponse(const Swift::JID jid, const std::string &str);
    void publishedDeviceList(const std::string& str);
    void publishedBundle(const std::string& str);
    void requestBundleHandler(const Swift::JID &jid, const std::string &bundleId, void *qMsg, const std::string& str);
    void pepBundleForKeytransport(const std::string from, const std::string& items);
    bool isEncryptedMessage(const QString& xmlNode);
    QString getSerializedStringFromMessage(Swift::Message::ref msg);

    Swift::Client* client_{};
    QString deviceListNodeName_{};
    QString myBareJid_{};
    QString namespace_{};
    char* uname_{nullptr};

    int uninstall_{0};
};
