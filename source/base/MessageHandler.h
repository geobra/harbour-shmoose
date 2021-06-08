#pragma once

#include <QObject>
#include <QStringList>
#include <Swiften/Swiften.h>
#include "Settings.h"

class DownloadManager;
class Persistence;
class ChatMarkers;
class RosterController;
class LurchAdapter;
class XMPPMessageParserClient;

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    MessageHandler(Persistence* persistence, Settings * settings, RosterController* rosterController, LurchAdapter* omemo, QObject *parent = 0);

    void setupWithClient(Swift::Client* client);

    void sendMessage(QString const &toJid, QString const &message, QString const &type, bool isGroup);
    void sendDisplayedForJid(const QString &jid);

signals:
    void messageSent(QString msgId);
    void messageReceived(QString fromJid, QString body);

public slots:
    void slotAppGetsActive(bool active);
    void sendRawMessageStanza(QString str);

private:
#ifdef DBUS
public:
#endif
    Swift::Client* client_;
    Persistence* persistence_;
    LurchAdapter* lurchAdapter_;
    Settings* settings_;

    DownloadManager* downloadManager_;
    ChatMarkers* chatMarkers_;

    XMPPMessageParserClient* xmppMessageParserClient_;

    bool appIsActive_;
    QStringList unAckedMessageIds_;

    void handleMessageReceived(Swift::Message::ref message);
    void handleStanzaAcked(Swift::Stanza::ref stanza);
    void handleDataReceived(Swift::SafeByteArray data);
};
