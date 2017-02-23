#ifndef MUCMANAGER_H
#define MUCMANAGER_H

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

    void addRoom(Swift::JID &jroomJid, QString const &RoomName);
    void removeRoom(QString const &jroomJid);

signals:
    void newGroupForContactsList(QString groupJid, QString groupName);
    void removeGroupFromContactsList(QString groupJid);

public slots:

private:
    Swift::Client* client_;
    Swift::MUCBookmarkManager *mucBookmarkManager_;

    void handleBookmarksReady();
    void handleBookmarkAdded(Swift::MUCBookmark bookmark);
    void handleBookmarkRemoved(Swift::MUCBookmark bookmark);

    void joinRoomIfConfigured(Swift::MUCBookmark const &bookmark);
    void sendUnavailableToRoom(Swift::MUCBookmark bookmark);

    QString getNickName();
};

#endif // MUCMANAGER_H
