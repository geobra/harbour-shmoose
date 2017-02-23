#include "MucManager.h"

#include <iostream>

MucManager::MucManager(QObject *parent) :
    QObject(parent), client_(NULL), mucBookmarkManager_(NULL)
{
}

MucManager::~MucManager()
{
    delete mucBookmarkManager_;
}

void MucManager::setClient(Swift::Client* client)
{
    client_ = client;
}

void MucManager::initialize()
{
    mucBookmarkManager_ = new Swift::MUCBookmarkManager(client_->getIQRouter());
    mucBookmarkManager_->onBookmarksReady.connect(boost::bind(&MucManager::handleBookmarksReady, this));
    mucBookmarkManager_->onBookmarkAdded.connect(boost::bind(&MucManager::handleBookmarkAdded, this, _1));
    mucBookmarkManager_->onBookmarkRemoved.connect(boost::bind(&MucManager::handleBookmarkRemoved, this, _1));
}

void MucManager::handleBookmarksReady()
{
    std::vector<Swift::MUCBookmark> bookmarks = mucBookmarkManager_->getBookmarks();
    //std::cout << "########MBM#####" <<  bookmarks.size() << std::endl;

    for(std::vector<Swift::MUCBookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
    {
        std::cout << "rooms: " << (*it).getRoom() << std::endl;
        emit newGroupForContactsList( QString::fromStdString((*it).getRoom().toBare().toString()) , "");

        // maybee join room
        joinRoomIfConfigured(*it);
    }
}

void MucManager::handleBookmarkAdded(Swift::MUCBookmark bookmark)
{
    std::cout << "handleBookmarkAdded: " << bookmark.getRoom().toBare().toString() << std::endl;

    // update contacts list
    emit newGroupForContactsList( QString::fromStdString(bookmark.getRoom().toBare().toString()) , QString::fromStdString(bookmark.getName()));

    // maybee join room
    joinRoomIfConfigured(bookmark);
}

void MucManager::joinRoomIfConfigured(Swift::MUCBookmark const &bookmark)
{
    // join room if autoJoin
    if (bookmark.getAutojoin())
    {
        Swift::MUC::ref muc = client_->getMUCManager()->createMUC(bookmark.getRoom());

        std::string nick = "";
        boost::optional<std::string> optionalNick = bookmark.getNick();
        if (optionalNick)
        {
            nick = *optionalNick;
        }
        else
        {
            nick = getNickName().toStdString();
        }
        muc->joinAs(nick);

        // FIXME
        // onJoinComplete
        // onJoinFailed
    }
}

QString MucManager::getNickName()
{
    // FIXME get name from settings page
    QString nick = QString::fromStdString(client_->getJID().toBare().toString());
    nick.replace("@", "<at>");

    return nick;
}

void MucManager::handleBookmarkRemoved(Swift::MUCBookmark bookmark)
{
    std::cout << "handleBookmarkRemoved: " << bookmark.getRoom().toString() << std::endl;

    // leave room
    sendUnavailableToRoom(bookmark);

    // update roster
    emit removeGroupFromContactsList( QString::fromStdString(bookmark.getRoom().toBare().toString()) );
}

void MucManager::addRoom(Swift::JID &jroomJid, QString const &RoomName)
{
    Swift::MUCBookmark mucBookmark(jroomJid, RoomName.toStdString());

    mucBookmark.setNick(getNickName().toStdString());
    mucBookmark.setAutojoin(true);

    mucBookmarkManager_->addBookmark(mucBookmark);
}

void MucManager::removeRoom(QString const &roomJid)
{
    std::vector< Swift::MUCBookmark > bookmarks = mucBookmarkManager_->getBookmarks();

    for(std::vector<Swift::MUCBookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
    {
        if ((*it).getRoom().toBare().toString().compare(roomJid.toStdString()) == 0)
        {
            mucBookmarkManager_->removeBookmark(*it);
            break;
        }
    }
}

void MucManager::sendUnavailableToRoom(Swift::MUCBookmark bookmark)
{
    Swift::Presence::ref presence = Swift::Presence::create();
    presence->setTo(bookmark.getRoom());
    presence->setType(Swift::Presence::Unavailable);
    Swift::PresenceSender *presenceSender = client_->getPresenceSender();
    presenceSender->sendPresence(presence);
}
