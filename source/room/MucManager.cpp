#include "MucManager.h"

#include <QDateTime>
#include <QDebug>
#include <iostream>

MucManager::MucManager(QObject *parent) :
    QObject(parent), client_(nullptr), mucBookmarkManager_(nullptr), triggerNewMucSignal_(true)
{
}

MucManager::~MucManager()
{
//    mucCollection_.clear();
//    delete mucBookmarkManager_;
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

        client_->getEntityCapsProvider()->onCapsChanged.connect(boost::bind(&MucManager::handleCapsChanged, this, _1));
    }
}

void MucManager::handleCapsChanged(const Swift::JID &jid)
{
    QString groupJid = QString::fromStdString(jid.toBare().toString());
    QString roomName = getRoomNameFromCaps(jid.toBare());

//    qDebug() << "handleCapsChanged:" << groupJid << "roomName:" << roomName << endl;

    if (roomName.isEmpty() == false)
    {
        for(auto bookmark : mucBookmarkManager_->getBookmarks())
        {
            if ((bookmark.getRoom().compare(jid, Swift::JID::WithoutResource) == 0) &&
                QString::fromStdString(bookmark.getName()) != roomName)
            {
//                qDebug() << "replace bookmark:" << groupJid << "room Name:" << QString::fromStdString(bookmark.getName()) << endl;

                std::shared_ptr<Swift::MUCBookmark> newBookmark(new Swift::MUCBookmark(jid.toBare(), roomName.toStdString()));
                newBookmark->setNick(bookmark.getNick());
                newBookmark->setAutojoin(bookmark.getAutojoin());

                mucBookmarkManager_->replaceBookmark(bookmark, *newBookmark);

                // the group is already in the roster, it will just update its name
                emit newGroupForContactsList(groupJid, roomName);
                break;
            }
        }
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
    std::cout << "###################### handleBookmarkAdded: ############### " << roomJid.toBare().toString() << ", name: " << bookmark.getName() << std::endl;

    // update contacts list
    if (roomJid.isValid())
    {
        emit newGroupForContactsList( QString::fromStdString(roomJid.toBare().toString()) , QString::fromStdString(bookmark.getName()));

        // maybee join room
        joinRoomIfConfigured(bookmark);
    }
}

QString MucManager::getRoomNameFromCaps(const Swift::JID &jid)
{
    auto discoInfo = client_->getEntityCapsProvider()->getCaps(jid.toBare());

    if(discoInfo != nullptr)
    {
        for(auto form : discoInfo->getExtensions())
        {
            auto field = form->getField("muc#roomconfig_roomname");

            if(field != nullptr)
            {
                auto values = field->getValues();

                if(values.empty() == false)
                {
                    return QString::fromStdString(values[0]);
                }
            }
        }
    }

    return "";
}

void MucManager::joinRoomIfConfigured(Swift::MUCBookmark const &bookmark)
{
    // join room if autoJoin
    if (bookmark.getAutojoin())
    {
        Swift::MUC::ref muc = client_->getMUCManager()->createMUC(bookmark.getRoom());
        muc->onJoinComplete.connect(boost::bind(&MucManager::handleJoinComplete, this, _1));
        muc->onJoinFailed.connect(boost::bind(&MucManager::handleJoinFailed, this, _1));

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
    nick.replace("@", "(at)");
    //QString nick = QString::fromStdString(client_->getJID().getNode());

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
//    std::shared_ptr<Swift::MUC> muc = client_->getMUCManager()->createMUC(roomJid);
//    muc->onJoinComplete.connect(boost::bind(&MucManager::handleJoinComplete, this, _1));
//    muc->onJoinFailed.connect(boost::bind(&MucManager::handleJoinFailed, this, _1));
    // save as bookmark if not already in
    if (isRoomAlreadyBookmarked(QString::fromStdString(roomJid)) == false)
    {
        // create bookmark
        std::shared_ptr<Swift::MUCBookmark> mucBookmark(new Swift::MUCBookmark(roomJid, roomName.toStdString()));
        mucBookmark->setNick(nickName);
        mucBookmark->setAutojoin(true);
        mucBookmarkManager_->addBookmark(*mucBookmark);

        // save MucCollection
//        std::shared_ptr<MucCollection> mucCollection(new MucCollection(muc, mucBookmark, nickName));
//        mucCollection_.push_back(mucCollection);
    }

    // try to join. onJoinComplete, add to bookmark
//  muc->joinAs(nickName);
}

void MucManager::handleJoinComplete(const std::string &joinedName)
{
    std::cout << "join complete: " << joinedName;

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
