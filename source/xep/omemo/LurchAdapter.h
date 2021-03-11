#pragma once

#include "ItemsPayloadParserFactory.h"
#include "ItemsPayloadSerializer.h"

#include "EncryptedPayloadParserFactory.h"
#include "EncryptedPayloadSerializer.h"

#include <QObject>
#include <QString>

#include <Swiften/Swiften.h>

class LurchAdapter : public QObject
{
    Q_OBJECT
public:
    explicit LurchAdapter(QObject *parent = nullptr);
    ~LurchAdapter();
    void setupWithClient(Swift::Client* client);
    QString getFeature();

    std::string messageEncryptIm(const std::string msg);
    bool exchangePlainBodyByOmemoStanzas(Swift::Message::ref msg);
    int decryptMessageIfEncrypted(Swift::Message::ref aMessage);

    void sendAsPepStanza(char* stz);
    void sendRawMessageStanza(char* stz);
    void sendBundleRequest(char* node, void *q_msg);
    void createAndSendBundleRequest(char* sender, char* bundle);
    void showMessageToUser(char *title, char *msg);

    void setCurrentChatPartner(const QString& jid);
    void callLurchCmd(const std::vector<std::string>& sl);
    bool isOmemoUser(const QString& bareJid);    

signals:
    void rawMessageStanzaForSending(QString);
    void signalReceivedDeviceListOfJid(QString);
    void signalShowMessage(QString, QString);

#ifdef UNIT_TEST
public:
#else
private:
#endif
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
    void handleMessageReceived(Swift::Message::ref message);

    Swift::Client* client_{};

    ItemsPayloadParserFactory itemsPayloadParserFactory_{};
    ItemsPayloadSerializer itemsPayloadSerializer_{};

    EncryptedPayloadParserFactory encryptedPayloadParserFactory_{};
    EncryptedPayloadSerializer encryptedPayloadSerializer_{};

    QString deviceListNodeName_{};
    QString myBareJid_{};
    QString namespace_{};
    char* uname_{nullptr};

    int uninstall_{0};
};
