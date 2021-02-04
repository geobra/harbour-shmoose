#include "RosterController.h"
#include "System.h"
#include "PresenceHandler.h"

#include <QQmlContext>
#include <QImage>
#include <QStandardPaths>
#include <QDir>

#include <algorithm>

#include <QDebug>

RosterController::RosterController(QObject *parent) : QObject(parent),
    client_(NULL), rosterList_(),
    presenceHandler_(new PresenceHandler(this))
{
    QString avatarLocation = System::getAvatarPath();
    QDir dir(avatarLocation);

    if (!dir.exists())
    {
        dir.mkpath(".");
    }
}

void RosterController::setupWithClient(Swift::Client *client)
{
    if (client != NULL)
    {
        client_ = client;

        Swift::XMPPRoster *xmppRoster = client_->getRoster();
        xmppRoster->onInitialRosterPopulated.connect(boost::bind(&RosterController::bindJidUpdateMethodes, this));

        client_->onMessageReceived.connect(boost::bind(&RosterController::handleMessageReceived, this, _1));

        presenceHandler_->setupWithClient(client_);
    }
}

/*
 * called after an contactAdd
 * iq get/set rosterItem
 */
void RosterController::handleJidAdded(const Swift::JID &jid)
{
    //qDebug() << "################# RosterController::handleJidAdded: " << QString::fromStdString(jid.toString());
    //dumpRosterList();

    if (isJidInRoster(QString::fromStdString(jid.toBare().toString())) == false)
    {
        rosterList_.append(new RosterItem(QString::fromStdString(jid.toBare().toString()),
                                          QString::fromStdString(client_->getRoster()->getNameForJID(jid)),
                                          RosterItem::SUBSCRIPTION_NONE, false, this));

        sortRosterList();

        // request subscription
        Swift::Presence::ref presence = Swift::Presence::create();
        presence->setTo(jid);
        presence->setType(Swift::Presence::Subscribe);
        client_->sendPresence(presence);

        emit rosterListChanged();
    }

    //qDebug() << "##################### handleJidAdded: rL_.size: " << rosterList_.size();
}

void RosterController::handleJidUpdated(const Swift::JID &jid, const std::string &name, const std::vector< std::string >& params)
{
    //std::cout << "############# RosterController::handleJidUpdated " << jid.toString() << ", name: " << name << std::endl;
    //dumpRosterList();

    for (auto item: params)
    {
        std::cout << "   params: " << item << std::endl;
    }

    bool changed1 = updateNameForJid(jid, name);

    Swift::XMPPRoster *xmppRoster = client_->getRoster();
    Swift::RosterItemPayload::Subscription subs = xmppRoster->getSubscriptionStateForJID(jid);

    bool changed2 = updateSubscriptionForJid(jid, static_cast<RosterItem::Subscription>(subs));

    if (changed1 || changed2)
    {
        emit rosterListChanged();
    }

    //qDebug() << "#####################handleJidUpdated: rL_.size: " << rosterList_.size();
}

bool RosterController::updateNameForJid(const Swift::JID &jid, const std::string &name)
{
    //qDebug()  << "-- updateNameForJid: " << QString::fromStdString(jid.toString()) << ", name: " << QString::fromStdString(name);
    bool somethingChanged = false;

    QString localBareJid = QString::fromStdString(jid.toBare().toString());
    appendToRosterIfNotAlreadyIn(localBareJid);

    for (auto item: rosterList_)
    {
        if (item->getJid().compare(localBareJid, Qt::CaseInsensitive) == 0)
        {
            item->setName(QString::fromStdString(name));
            somethingChanged = true;
            break;
        }
    }

    return somethingChanged;
}

bool RosterController::updateSubscriptionForJid(const Swift::JID &jid, RosterItem::Subscription subscription)
{
    //qDebug()  << "-- updateSubscriptionForJid: " << QString::fromStdString(jid.toString()) << ", subs: " << subscription;

    bool somethingChanged = false;

    QString localBareJid = QString::fromStdString(jid.toBare().toString());
    appendToRosterIfNotAlreadyIn(localBareJid);

    for (auto item: rosterList_)
    {
        if (item->getJid().compare(localBareJid, Qt::CaseInsensitive) == 0)
        {
            item->setSubscription(subscription);

            // FIXME need two signal here for testing?!
            emit rosterListChanged();
            emit subscriptionUpdated(subscription);

            somethingChanged = true;
            break;
        }
    }

    return somethingChanged;
}

bool RosterController::updateStatusForJid(const Swift::JID &jid, const QString& status)
{
    //qDebug()  << "-- updateStatusForJid: " << QString::fromStdString(jid.toString()) << ", status: " << status;

    bool somethingChanged = false;

    QString localBareJid = QString::fromStdString(jid.toBare().toString());
    appendToRosterIfNotAlreadyIn(localBareJid);

    if (! status.isEmpty())
    {
        for (auto item: rosterList_)
        {
            if (item->getJid().compare(localBareJid, Qt::CaseInsensitive) == 0)
            {
                item->setStatus(status);
                emit rosterListChanged();

                somethingChanged = true;
                break;
            }
        }
    }

    return somethingChanged;
}

bool RosterController::updateAvailabilityForJid(const Swift::JID &jid, const RosterItem::Availability& availability)
{
    //qDebug()  << "-- updateAvailabilityForJid: " << QString::fromStdString(jid.toString()) << ", ava: " << availability;

    bool somethingChanged = false;

    QString localBareJid = QString::fromStdString(jid.toBare().toString());
    appendToRosterIfNotAlreadyIn(localBareJid);

    for (auto item: rosterList_)
    {
        if (item->getJid().compare(localBareJid, Qt::CaseInsensitive) == 0)
        {
            item->setAvailability(availability);
            emit rosterListChanged();

            somethingChanged = true;
            break;
        }
    }

    return somethingChanged;
}


void RosterController::handleUpdateFromPresence(const Swift::JID &jid, const QString &status, const RosterItem::Availability& availability)
{
    //std::cout << "########### RosterController::handleUpdatedFromPresence " << jid.toString() << ", status: " << status.toStdString() << std::endl;
    //dumpRosterList();

    bool changed1 = updateStatusForJid(jid, status);
    bool changed2 = updateAvailabilityForJid(jid, availability);

    if (changed1 || changed2)
    {
        emit rosterListChanged();
    }

    //qDebug() << "#####################handleUpdateFromPresence: rL_.size: " << rosterList_.size();
}

void RosterController::handleJidRemoved(const Swift::JID &jid)
{
    //std::cout << "############ RosterController::handleJidRemoved: " << jid.toString() << std::endl;
    //dumpRosterList();

    bool somethingChanged = false;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    while (it != rosterList_.end())
    {
        if ((*it)->getJid() == QString::fromStdString(jid.toBare().toString()))
        {
            it = rosterList_.erase(it);
            // TODO what about deleting the RosterItem object?!

            somethingChanged = true;
            break;
        }
        else
        {
            ++it;
        }
    }

    if (somethingChanged)
    {
        //qDebug() << "handleJidRemoved: emit rosterListChanged";

        sortRosterList();
        emit rosterListChanged();
    }

    //qDebug() << "#####################handleJidRemoved: rL_.size: " << rosterList_.size();
}

void RosterController::handleMessageReceived(Swift::Message::ref message)
{
    if (message->getType() == Swift::Message::Groupchat)
    {
        // check for updated room name
        std::string roomName = message->getSubject();

        if (! roomName.empty() )
        {
            this->updateNameForJid(message->getFrom().toBare(), roomName);
        }
    }
}

void RosterController::requestRoster()
{
    client_->requestRoster();

    Swift::GetRosterRequest::ref rosterRequest = Swift::GetRosterRequest::create(client_->getIQRouter());
    rosterRequest->onResponse.connect(bind(&RosterController::handleRosterReceived, this, _2));
    rosterRequest->send();
}

/*
 * called after the requested roster list is received.
 * Attention. In real world updated from presence will arrive before the roster is received.
 */
void RosterController::handleRosterReceived(Swift::ErrorPayload::ref error)
{
    //qDebug() << "handleRosterReceived";
    //dumpRosterList();

    if (error)
    {
        std::cerr << "Error receiving roster. Continuing anyway.";
    }
    else
    {
        bool somethingChanged = false;

        Swift::VCardManager *vCardManager = client_->getVCardManager();
        vCardManager->onVCardChanged.connect(boost::bind(&RosterController::handleVCardChanged, this, _1, _2));

        Swift::XMPPRoster* roster = client_->getRoster();
        std::vector<Swift::XMPPRosterItem> rosterItems = roster->getItems();

        std::vector<Swift::XMPPRosterItem>::iterator it;

        for(it = rosterItems.begin(); it < rosterItems.end(); it++ )
        {
            vCardManager->requestVCard((*it).getJID());

            QString bareJid = QString::fromStdString((*it).getJID().toBare().toString());
            if (isJidInRoster(bareJid) == false)
            {
                //qDebug() << "######## handleRosterReceived: append " << bareJid;
                rosterList_.append(new RosterItem(QString::fromStdString((*it).getJID().toBare().toString()),
                                                  QString::fromStdString((*it).getName()),
                                                  (RosterItem::Subscription)(*it).getSubscription(), false, this));

                somethingChanged = true;
            }
            else
            {
                updateNameForJid((*it).getJID(), (*it).getName());
                updateSubscriptionForJid((*it).getJID(), (RosterItem::Subscription)(*it).getSubscription());
            }
        }

        if (somethingChanged)
        {
            sortRosterList();
            emit rosterListChanged();
        }
    }

    //qDebug() << "#####################handleRosterReceived: rL_.size: " << rosterList_.size();
}

void RosterController::handleVCardChanged(const Swift::JID &jid, const Swift::VCard::ref &vCard)
{
    Swift::VCardManager *vCardManager = client_->getVCardManager();

    const QString bareJid = QString::fromStdString(jid.toBare().toString());
    const QString newHash = QString::fromStdString(vCardManager->getPhotoHash(jid));

    if (checkHashDiffers(bareJid, newHash) == true && vCard)
    {
        Swift::ByteArray imageByteArray = vCard->getPhoto();

        // FIXME is there an easier way from Swift::ByteArray to QByteArray?!
        QByteArray byteArray;
        for (std::vector<unsigned char>::iterator it = imageByteArray.begin(); it != imageByteArray.end(); it++)
        {
            byteArray.append(*it);
        }

        // try to extract photo type for processing to QImage
        QString photoType = QString::fromStdString(vCard->getPhotoType());
        QString imageType = photoType;
        if (imageType.contains("/"))
        {
            QStringList splitedImageType = imageType.split("/");
            if (splitedImageType.size() == 2)
            {
                imageType = splitedImageType.at(1);
            }
        }

        // create a QImage out of the vCard photo data
        QImage image = QImage::fromData(byteArray, imageType.toStdString().c_str());

        if (image.isNull() == false)
        {
            // write image file to disk, try to delete old one
            QString imageName = bareJid;
            imageName.replace("@", "-at-");
            QString imageBasePath = System::getAvatarPath() + QDir::separator() + imageName;
            QString imagePath = imageBasePath + ".png";

            QFile oldImageFile(imagePath);
            oldImageFile.remove();

            if (! image.save(imagePath, "PNG"))
            {
                qDebug() << "cant save image to: " << imagePath;
            }

            // write hash file to disk, try to delete old one
            QString imageHash = imageBasePath + ".hash";
            QFile hashFile(imageHash);
            hashFile.remove();
            if (hashFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&hashFile);
                out << newHash;
                hashFile.close();
            }
            else
            {
                qDebug() << "cant save hash to: " << imageHash;
            }

            // signal new avatar to the rosterItem
            foreach(RosterItem *item, rosterList_)
            {
                if (item->getJid().compare(bareJid) == 0)
                {
                    item->triggerNewImage();
                }
            }
        }
    }
}

bool RosterController::checkHashDiffers(QString const &jid, QString const &newHash)
{
    bool returnValue = false;

    // read existing hash from file
    QString jidPart = jid;
    jidPart.replace("@", "-at-");

    QString imageHashPath = System::getAvatarPath() + QDir::separator() + jidPart + ".hash";
    QFileInfo info(imageHashPath);

    if (info.exists() && info.isFile())
    {
        QFile file(imageHashPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString line = file.readLine();
            if (line.compare(newHash) == 0)
            {
                // same hash
                returnValue = false;
            }
            else
            {
                // hash differs
                returnValue = true;
            }

            //std::cout << "old hash: " << line.toStdString() << ". new: " << newHash.toStdString() << "bool: " << returnValue << std::endl;

            file.close();
        }
        else
        {
            qDebug() << "cant save hash to: " << imageHashPath;
        }

    }
    else
    {
        // no hash file. hash differs!
        returnValue = true;
    }

    return returnValue;
}

void RosterController::bindJidUpdateMethodes()
{
    Swift::XMPPRoster *xmppRoster = client_->getRoster();
    xmppRoster->onJIDAdded.connect(boost::bind(&RosterController::handleJidAdded, this, _1));
    xmppRoster->onJIDRemoved.connect(boost::bind(&RosterController::handleJidRemoved, this, _1));
    xmppRoster->onJIDUpdated.connect(boost::bind(&RosterController::handleJidUpdated, this, _1, _2, _3));
}

void RosterController::addContact(const QString& jid, const QString& name)
{
    qDebug() << "addContact: " << jid << ", name: " << name;
    Swift::JID newContactJid(jid.toStdString());

    if (newContactJid.isValid() == true && isJidInRoster(jid) == false)
    {
        Swift::IDGenerator idGenerator;
        std::string msgId = idGenerator.generateID();
        std::shared_ptr<Swift::RosterPayload> payload(new Swift::RosterPayload);

        Swift::RosterItemPayload riPayload;
        riPayload.setJID(newContactJid);
        riPayload.setName(name.toStdString());
        riPayload.setSubscription(Swift::RosterItemPayload::None);
        payload->addItem(riPayload);

        Swift::IQRouter *iqRouter = client_->getIQRouter();
        iqRouter->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, Swift::JID(), msgId, payload));

        qDebug() << "  stanza sent";
    }
    else
    {
        emit signalShowMessage("Add Contact", "JID not valid or already in Roster!");
        qDebug() << "  already in roster";
    }
}

void RosterController::removeContact(const QString& jid)
{
    sendUnavailableAndUnsubscribeToJid(jid);

    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();
    std::shared_ptr<Swift::RosterPayload> payload(new Swift::RosterPayload);
    payload->addItem(Swift::RosterItemPayload(Swift::JID(jid.toStdString()), "", Swift::RosterItemPayload::Remove));
    Swift::IQRouter *iqRouter = client_->getIQRouter();
    iqRouter->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, Swift::JID(), msgId, payload));
}

void RosterController::sendUnavailableAndUnsubscribeToJid(const QString& jid)
{
    Swift::JID contactJid(jid.toStdString());

    if (contactJid.isValid())
    {
        // unavailable
        Swift::Presence::ref presenceUnavailable = Swift::Presence::create();
        presenceUnavailable->setTo(contactJid);
        presenceUnavailable->setType(Swift::Presence::Unavailable);
        client_->sendPresence(presenceUnavailable);

        // unsubscribe
        Swift::Presence::ref presenceUnsubscribe = Swift::Presence::create();
        presenceUnsubscribe->setTo(contactJid);
        presenceUnsubscribe->setType(Swift::Presence::Unsubscribe);
        client_->sendPresence(presenceUnsubscribe);
    }
}

QQmlListProperty<RosterItem> RosterController::getRosterList()
{
    return QQmlListProperty<RosterItem>(this, rosterList_);
}

bool RosterController::isJidInRoster(const QString& bareJid)
{
    bool returnValue = false;

    for(auto item: rosterList_)
    {
        if (item->getJid().compare(bareJid, Qt::CaseInsensitive) == 0)
        {
            returnValue = true;
            break;
        }
    }

    return returnValue;
}

void RosterController::addGroupAsContact(QString groupJid, QString groupName)
{
    //qDebug() << "addGroupAsContact";
    //dumpRosterList();

    if (isJidInRoster(groupJid) == false)
    {
        rosterList_.append(new RosterItem(groupJid, groupName, RosterItem::SUBSCRIPTION_NONE, true, this));

        sortRosterList();
        emit rosterListChanged();
    }
    else
    {
        //qDebug() << "############ group already in roster gui!" << groupJid;
    }

    //qDebug() << "#####################addGroupAsContact: rL_.size: " << rosterList_.size();
}

void RosterController::removeGroupFromContacts(QString groupJid)
{
    //qDebug() << "removeGroupFromContacts";
    //dumpRosterList();

    bool somethingChanged = false;
    QList<RosterItem*>::iterator it = rosterList_.begin();

    for (; it != rosterList_.end(); ++it)
    {
        if (groupJid.compare( (*it)->getJid() ) == 0)
        {
            qDebug() << "remove group:" << groupJid;

            it = rosterList_.erase(it);
            // TODO what about free the RosterItem?!

            somethingChanged = true;
            break;
        }
    }

    if (somethingChanged)
    {
        sortRosterList();
        //emit rosterListChanged();
    }

    //qDebug() << "#####################removeGroupFromContacts: rL_.size: " << rosterList_.size();
}

bool RosterController::isGroup(QString const &jid)
{
    bool returnValue = true;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if (jid.compare((*it)->getJid()) == 0)
        {
            returnValue = (*it)->isGroup();
            break;
        }
    }

    return returnValue;
}

QString RosterController::getAvatarImagePathForJid(QString const &jid)
{
    return getTypeForJid(attributePicturePath, jid);
}

QString RosterController::getNameForJid(QString const &jid)
{
    return getTypeForJid(attributeName, jid);
}



QString RosterController::getTypeForJid(itemAttribute const &attribute, QString const &jid)
{
    QString returnValue = "";

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if (jid.compare((*it)->getJid()) == 0)
        {
            if (attribute == attributePicturePath)
            {
                returnValue = (*it)->getImagePath();
            }
            else if(attribute == attributeName)
            {
                returnValue = (*it)->getName();
            }
            break;
        }
    }

    return returnValue;
}

void RosterController::appendToRosterIfNotAlreadyIn(const QString& jid)
{
    //qDebug() << "----------------- appendToRosterIfNotAlreadyIn ----------  " << jid;
    if (isJidInRoster(jid) == false // not already in
            &&
            (! jid.compare( QString::fromStdString(client_->getJID().toBare().toBare()), Qt::CaseInsensitive) == 0)) // not the user of this client instance
    {
        rosterList_.append(new RosterItem(jid, jid, RosterItem::SUBSCRIPTION_NONE, false, this));
    }
}

void RosterController::sortRosterList()
{
    struct
    {
        bool operator()(RosterItem* a, RosterItem* b)
        {
            return a->getName() < b->getName();
        }
    } customSort;

    std::sort(rosterList_.begin(), rosterList_.end(), customSort);
}

#ifdef DBUS
QList<RosterItem*> RosterController::fetchRosterList()
{
    return rosterList_;
}
#endif

void RosterController::dumpRosterList()
{
    for (auto item: rosterList_)
    {
        qDebug() << "rl: " << item->getJid() << ", name: " << item->getName() << ", isGroup? " << item->isGroup();
    }
}
