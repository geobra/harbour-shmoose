#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <Swiften/Swiften.h>

class DownloadManager;
class Persistence;
class ChatMarkers;
class Omemo;
class XMPPMessageParserClient;

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(Persistence* persistence, QObject *parent = 0);
    ~MessageHandler();

    void setupWithClient(Swift::Client* client);

    void sendMessage(QString const &toJid, QString const &message, QString const &type, bool isGroup);
    void sendDisplayedForJid(const QString &jid);

signals:

public slots:
    void slotAppGetsActive(bool active);

private:
    Swift::Client* client_;
    Persistence* persistence_;

    DownloadManager* downloadManager_;
    ChatMarkers* chatMarkers_;

    Omemo* omemo_;
    XMPPMessageParserClient* xmppMessageParserClient_;

    bool appIsActive_;
    QStringList unAckedMessageIds_;

    void handleMessageReceived(Swift::Message::ref message);
    void handleStanzaAcked(Swift::Stanza::ref stanza);

    QString getSerializedStringFromMessage(Swift::Message::ref msg);
    bool isEncryptedMessage(const QString& xmlNode);
};

#endif // MESSAGEHANDLER_H
