#ifndef CHATMARKERS_H
#define CHATMARKERS_H

#include <QObject>

#include <Swiften/Swiften.h>

class Persistence;

class ChatMarkers : public QObject
{
    Q_OBJECT
public:
    explicit ChatMarkers(Persistence* persistence, QObject *parent = 0);
    void setupWithClient(Swift::Client *client);

    static QString getMarkableString();
    QString getDisplayedStringForId(QString displayedId);

    void sendDisplayedForJidAndMessageId(QString jid, QString messageId);
    void sendDisplayedForJid(const QString &jid);

    static const QString chatMarkersIdentifier;

signals:

public slots:

private:
    void handleMessageReceived(Swift::Message::ref message);
    QString getIdFromRawXml(QString rawXml);

    Swift::Client* client_;
    Persistence* persistence_;
};

#endif // CHATMARKERS_H
