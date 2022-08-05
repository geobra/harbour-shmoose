#ifndef CHATMARKERS_H
#define CHATMARKERS_H

#include <QObject>

#include <Swiften/Swiften.h>

class Persistence;
class RosterController;

class ChatMarkers : public QObject
{
    Q_OBJECT
public:
    ChatMarkers(Persistence* persistence, RosterController* rosterController, QObject *parent = 0);
    void setupWithClient(Swift::Client *client);

    static QString getMarkableString();

    void sendDisplayedForJidAndMessageId(QString jid, QString messageId);
    void sendDisplayedForJid(const QString &jid);

    void handleMessageReceived(Swift::Message::ref message);

    static const QString chatMarkersIdentifier;

signals:

public slots:

private:
#if defined DBUS || defined UNIT_TEST
public:
#endif
    QString getIdFromRawXml(QString rawXml);

    Swift::Client* client_;
    Persistence* persistence_;
    RosterController* rosterController_;
};

#endif // CHATMARKERS_H
