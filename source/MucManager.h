#ifndef MUCMANAGER_H
#define MUCMANAGER_H

#include "MucCollection.h"

#include <QObject>
#include <Swiften/Swiften.h>

class MucManager : public QObject
{
    Q_OBJECT

public:
    explicit MucManager(QObject *parent = 0);
    ~MucManager();

    void setClient(Swift::Client* client);
    void initialize();

    void addRoom(Swift::JID &roomJid, QString const &roomName);
    void removeRoom(QString const &jroomJid);

signals:
    void newGroupForContactsList(QString groupJid, QString groupName);
    void removeGroupFromContactsList(QString groupJid);

    void signalShowMessage(QString headline, QString body);

private:
    Swift::Client* client_;
    Swift::MUCBookmarkManager *mucBookmarkManager_;
    std::vector<boost::shared_ptr<MucCollection>> mucCollection_;

    void handleBookmarksReady();
    void handleBookmarkAdded(Swift::MUCBookmark bookmark);
    void handleBookmarkRemoved(Swift::MUCBookmark bookmark);

    void handleJoinFailed(Swift::ErrorPayload::ref error);
    void handleJoinComplete(const std::string &joinedName);

    void handleMessageReceived(Swift::Message::ref message);

    void joinRoomIfConfigured(Swift::MUCBookmark const &bookmark);
    void sendUnavailableToRoom(Swift::MUCBookmark bookmark);

    QString getNickName();
};

#endif // MUCMANAGER_H
