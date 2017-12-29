#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <Swiften/Swiften.h>

class DownloadManager;
class Persistence;
class ChatMarkers;

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = 0);

    void setClient(Swift::Client* client);
    void setPersistence(Persistence* persistence);
    void initialize();

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

    bool appIsActive_;
    QStringList unAckedMessageIds_;

    void handleMessageReceived(Swift::Message::ref message);
    void handleStanzaAcked(Swift::Stanza::ref stanza);
};

#endif // MESSAGEHANDLER_H
