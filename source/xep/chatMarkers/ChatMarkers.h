#ifndef CHATMARKERS_H
#define CHATMARKERS_H

#include <QObject>

#include <Swiften/Swiften.h>

class Persistence;

class ChatMarkers : public QObject
{
    Q_OBJECT
public:
    explicit ChatMarkers(QObject *parent = 0);
    void setClient(Swift::Client *client);
    void setPersistence(Persistence* persistence);
    void initialize();

    QString getMarkableString();
    QString getDisplayedStringForId(QString displayedId);

    void sendDisplayedForJidAndMessageId(QString jid, QString messageId);
    void sendDisplayedForJid(QString jid);

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
