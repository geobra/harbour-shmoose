#include "MucManager.h"

#include <iostream>

MucManager::MucManager(QObject *parent) :
    QObject(parent), client_(NULL), mucBookmarkManager_(NULL), triggerNewMucSignal_(true)
{
}

MucManager::~MucManager()
{
    mucCollection_.clear();
    delete mucBookmarkManager_;
}

void MucManager::setupWithClient(Swift::Client* client)
{
    if (client != NULL)
    {
        client_ = client;

        mucBookmarkManager_ = new Swift::MUCBookmarkManager(client_->getIQRouter());
        mucBookmarkManager_->onBookmarksReady.connect(boost::bind(&MucManager::handleBookmarksReady, this));
        mucBookmarkManager_->onBookmarkAdded.connect(boost::bind(&MucManager::handleBookmarkAdded, this, _1));
        mucBookmarkManager_->onBookmarkRemoved.connect(boost::bind(&MucManager::handleBookmarkRemoved, this, _1));

        client_->onMessageReceived.connect(boost::bind(&MucManager::handleMessageReceived, this, _1));

        client_->onConnected.connect(boost::bind(&MucManager::handleConnected, this));
    }
}

void MucManager::handleConnected()
{
    std::cout << "##################### MucManager::handleConnected() -> rejoin rooms ######################" << std::endl;

    // This is only needed on a reconnt after an network loss.
    // It will rejoin to the rooms from the bookmarks.
    // It does nothing on a first connection.

    triggerNewMucSignal_ = false;
    handleBookmarksReady();
    triggerNewMucSignal_ = true;
}

void MucManager::handleMessageReceived(Swift::Message::ref message)
{
    // Examples/MUCListAndJoin/MUCListAndJoin.cpp
    if (message->getPayload<Swift::MUCInvitationPayload>())
    {
        //qDebug() << "its a muc inventation!!!";
        Swift::MUCInvitationPayload::ref mucInventation = message->getPayload<Swift::MUCInvitationPayload>();

        Swift::JID roomJid = mucInventation->getJID();
        QString roomName = QString::fromStdString(message->getSubject());

        this->addRoom(roomJid, roomName);
    }
}

void MucManager::handleBookmarksReady()
{
    std::cout << "##################### handleBookmarksReady ######################" << std::endl;
    std::vector<Swift::MUCBookmark> bookmarks = mucBookmarkManager_->getBookmarks();

    for(std::vector<Swift::MUCBookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
    {
        Swift::JID roomJid((*it).getRoom());
        std::cout << "rooms: jid:" << roomJid << ", name: " << (*it).getName() << std::endl;

        if (roomJid.isValid())
        {
            if (triggerNewMucSignal_ == true)
            {
                emit newGroupForContactsList( QString::fromStdString(roomJid.toBare().toString()) , QString::fromStdString((*it).getName()));
            }

            // maybee join room
            joinRoomIfConfigured(*it);
        }
    }
}

void MucManager::handleBookmarkAdded(Swift::MUCBookmark bookmark)
{
    Swift::JID roomJid(bookmark.getRoom());

    //std::cout << "###################### handleBookmarkAdded: ############### " << roomJid.toBare().toString() << ", name: " << bookmark.getName() << std::endl;

    // update contacts list
    if (roomJid.isValid())
    {
        emit newGroupForContactsList( QString::fromStdString(roomJid.toBare().toString()) , QString::fromStdString(bookmark.getName()));

        // maybee join room
        joinRoomIfConfigured(bookmark);
    }
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

void MucManager::addRoom(Swift::JID &roomJid, QString const &roomName)
{
    std::string nickName = getNickName().toStdString();

    // create MUC
    boost::shared_ptr<Swift::MUC> muc = client_->getMUCManager()->createMUC(roomJid);
    muc->onJoinComplete.connect(boost::bind(&MucManager::handleJoinComplete, this, _1));
    muc->onJoinFailed.connect(boost::bind(&MucManager::handleJoinFailed, this, _1));

    // create bookmark
    boost::shared_ptr<Swift::MUCBookmark> mucBookmark(new Swift::MUCBookmark(roomJid, roomName.toStdString()));
    mucBookmark->setNick(nickName);
    mucBookmark->setAutojoin(true);

    // save MucCollection
    boost::shared_ptr<MucCollection> mucCollection(new MucCollection(muc, mucBookmark, nickName));
    mucCollection_.push_back(mucCollection);

    // try to join. onJoinComplete, add to bookmark
    muc->joinAs(getNickName().toStdString());
}

void MucManager::handleJoinComplete(const std::string &joinedName)
{
    std::cout << "join complete: " << joinedName;

    for(std::vector<boost::shared_ptr<MucCollection>>::iterator it = mucCollection_.begin(); it != mucCollection_.end(); ++it)
    {
        if ((*it)->getNickname().compare(joinedName) == 0)
        {
            boost::shared_ptr<Swift::MUCBookmark> bookmark = (*it)->getBookmark();
            if (bookmark)
            {
                mucBookmarkManager_->addBookmark(*bookmark);
                break;
            }
        }
    }
}

void MucManager::handleJoinFailed(Swift::ErrorPayload::ref error)
{
    if (error)
    {
        Swift::ErrorPayload joinError = *error;
        //std::cout << "join error: " << joinError.getText() << std::endl;
        signalShowMessage("Error joining room", QString::fromStdString(joinError.getText()));
    }
}

void MucManager::handleUserLeft(Swift::MUC::LeavingType lt)
{
    std::cout << "##################### MucManager::handleUserLeft:" << lt << std::endl;
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
    client_->sendPresence(presence);
}
