#include "MucManager.h"

#include <QDateTime>
#include <QDebug>
#include <iostream>

#include "XmlProcessor.h"
#include "MucRequest.h"

MucManager::MucManager(QObject *parent) :
    QObject(parent), client_(nullptr), mucBookmarkManager_(nullptr), triggerNewMucSignal_(true), nickName_("")
{
}

MucManager::~MucManager()
{
    mucCollection_.clear();
    delete mucBookmarkManager_;
}

void MucManager::setupWithClient(Swift::Client* client)
{
    if (client != nullptr)
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
        qDebug() << "its a muc inventation!!!";
        Swift::MUCInvitationPayload::ref mucInventation = message->getPayload<Swift::MUCInvitationPayload>();

        Swift::JID roomJid = mucInventation->getJID();

        // request room info to get room name
        MucRequest::ref mucRequest = MucRequest::create(roomJid.toString(), client_->getIQRouter());
        mucRequest->onResponse.connect(boost::bind(&MucManager::handleDiscoInfoResponse, this, _1, _2, _3));
        mucRequest->send();
    }
}

void MucManager::handleDiscoInfoResponse(const std::string& jid, std::shared_ptr<Swift::DiscoInfo> info, Swift::ErrorPayload::ref error)
{
    if (error == nullptr) {
        for (auto&& extension : info->getExtensions())
        {
            auto field = extension->getField("muc#roomconfig_roomname");

            if(field != nullptr)
            {
                addRoom(Swift::JID(jid), QString::fromStdString(field->getTextSingleValue()));
                break;
            }
        }
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

bool MucManager::isRoomAlreadyBookmarked(const QString& roomJid)
{
    qDebug() << "isRoomAlreadyBookmarked?: " << roomJid;
    bool returnValue = false;

    std::vector<Swift::MUCBookmark> bookmarks = mucBookmarkManager_->getBookmarks();

    for(std::vector<Swift::MUCBookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
    {
        Swift::JID roomJidBm((*it).getRoom());

        if (roomJid.compare(QString::fromStdString(roomJidBm.toBare().toString()), Qt::CaseInsensitive) == 0)
        {
            returnValue = true;
            break;
        }
    }

    qDebug() << "... " << returnValue;

    return returnValue;
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
    QString nick = QString::fromStdString(client_->getJID().toBare().toString());

    if(!nickName_.isEmpty())
        nick = nickName_;

    nick.replace("@", "(at)");

    return nick;
}

void MucManager::setNickName(QString const &NickName)
{
    nickName_ = NickName;
}

void MucManager::handleBookmarkRemoved(Swift::MUCBookmark bookmark)
{
    std::cout << "handleBookmarkRemoved: " << bookmark.getRoom().toString() << std::endl;

    //update mucCollection_
    for(std::vector<std::shared_ptr<MucCollection>>::iterator it = mucCollection_.begin(); it != mucCollection_.end(); ++it)
    {
        if ((*it)->getBookmark()->getRoom() == bookmark.getRoom())
        {
            mucCollection_.erase(it);
            break;
        }
    }

    // leave room
    sendUnavailableToRoom(bookmark);

    // update roster
    emit removeGroupFromContactsList( QString::fromStdString(bookmark.getRoom().toBare().toString()) );
}

void MucManager::addRoom(const Swift::JID &roomJid, QString const &roomName)
{
    std::string nickName = getNickName().toStdString();

    qDebug() << "add room " << QString::fromStdString(roomJid.toString()) << ", " << roomName << endl;

    // create MUC
    std::shared_ptr<Swift::MUC> muc = client_->getMUCManager()->createMUC(roomJid);
    muc->onJoinComplete.connect(boost::bind(&MucManager::handleJoinComplete, this, _1));
    muc->onJoinFailed.connect(boost::bind(&MucManager::handleJoinFailed, this, _1));

    // save as bookmark if not already in
    if (isRoomAlreadyBookmarked(QString::fromStdString(roomJid.toString())) == false)
    {
        // create bookmark
        std::shared_ptr<Swift::MUCBookmark> mucBookmark(new Swift::MUCBookmark(roomJid, roomName.toStdString()));
        mucBookmark->setNick(nickName);
        mucBookmark->setAutojoin(true);

        // save MucCollection
        std::shared_ptr<MucCollection> mucCollection(new MucCollection(muc, mucBookmark, nickName));
        mucCollection_.push_back(mucCollection);
    }

    // try to join. onJoinComplete, add to bookmark
    muc->joinAs(nickName);
}

void MucManager::renameRoom(QString const &roomJid, QString const &roomName)
{
    std::vector<Swift::MUCBookmark> bookmarks = mucBookmarkManager_->getBookmarks();

    for(std::vector<Swift::MUCBookmark>::iterator it = bookmarks.begin(); it != bookmarks.end(); ++it)
    {
        Swift::JID roomJidBm((*it).getRoom());

        if (roomJid.compare(QString::fromStdString(roomJidBm.toBare().toString()), Qt::CaseInsensitive) == 0)
        {
            if(QString::fromStdString((*it).getName()).compare(roomName, Qt::CaseInsensitive) != 0 )
            {
                std::shared_ptr<Swift::MUCBookmark> mucBookmark(new Swift::MUCBookmark(Swift::JID(roomJid.toStdString()), roomName.toStdString()));
                mucBookmark->setNick((*it).getNick());
                mucBookmark->setPassword((*it).getPassword());
                mucBookmark->setAutojoin((*it).getAutojoin());

                mucBookmarkManager_->replaceBookmark((*it), (*mucBookmark));
                // What needs to be done with mucCollection??
            }
            emit newGroupForContactsList(roomJid, roomName);
        }
    }
}

void MucManager::handleJoinComplete(const std::string &joinedName)
{
    std::cout << "join complete: " << joinedName;

    for(std::vector<std::shared_ptr<MucCollection>>::iterator it = mucCollection_.begin(); it != mucCollection_.end(); ++it)
    {
        if ((*it)->getNickname().compare(joinedName) == 0)
        {
            std::shared_ptr<Swift::MUCBookmark> bookmark = (*it)->getBookmark();
            // because the same nickname can be used for multiple rooms, check if the room is not already bookmarked
            if (bookmark && isRoomAlreadyBookmarked(QString::fromStdString(bookmark->getRoom())) == false)
            {
                mucBookmarkManager_->addBookmark(*bookmark);
                break;
            }
        }
    }

    emit roomJoinComplete(QString::fromStdString(joinedName));
}

void MucManager::handleJoinFailed(Swift::ErrorPayload::ref error)
{
    if (error)
    {
        Swift::ErrorPayload joinError = *error;
        //std::cout << "join error: " << joinError.getText() << std::endl;
        emit signalShowMessage("Error joining room", QString::fromStdString(joinError.getText()));
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