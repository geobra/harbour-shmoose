#include "RosterContoller.h"
#include "System.h"

#include <QQmlContext>
#include <QImage>
#include <QStandardPaths>
#include <QDir>

#include <QDebug>

RosterController::RosterController(QObject *parent) : QObject(parent), client_(NULL), rosterList_()
{
    QString avatarLocation = System::getAvatarPath();
    QDir dir(avatarLocation);

    if (!dir.exists())
    {
        dir.mkpath(".");
    }
}

void RosterController::setClient(Swift::Client *client)
{
    client_ = client;

    Swift::XMPPRoster *xmppRoster = client_->getRoster();
    xmppRoster->onInitialRosterPopulated.connect(boost::bind(&RosterController::bindJidUpdateMethodes, this));
}

void RosterController::handleJidAdded(const Swift::JID &jid)
{
    std::cout << "RosterController::handleJidAdded: " << jid.toString() << std::endl;

    rosterList_.append(new RosterItem(QString::fromStdString(jid.toBare().toString()),
                                      "", // FIXME set name
                                      RosterItem::SUBSCRIPTION_NONE, false, this));

    // request subscription
    Swift::Presence::ref presence = Swift::Presence::create();
    presence->setTo(jid);
    presence->setType(Swift::Presence::Subscribe);
    Swift::PresenceSender *presenceSender = client_->getPresenceSender();
    presenceSender->sendPresence(presence);

    emit rosterListChanged();
}

void RosterController::handleJidUpdated(const Swift::JID &jid, const std::string &name, const std::vector< std::string >&)
{
    std::cout << "RosterController::handleJidUpdated " << jid.toString() << ", " << name;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if ((*it)->getJid() == QString::fromStdString(jid.toBare().toString()))
        {
            (*it)->setName(QString::fromStdString(name));
        }
    }

   emit rosterListChanged();
}

void RosterController::handleUpdateFromPresence(const Swift::JID &jid, const QString &status, const RosterItem::Availability& availability)
{
    std::cout << "RosterController::handleUpdatedFromPresence " << jid.toString() << ", status: " << status.toStdString() << std::endl;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        //qDebug() << (*it)->getJid() << "<->" << QString::fromStdString(jid.toBare().toString());

        if ((*it)->getJid() == QString::fromStdString(jid.toBare().toString()))
        {
            (*it)->setAvailability(availability);

            if (! status.isEmpty())
            {
                (*it)->setStatus(status);
            }

            break;
        }
    }

   emit rosterListChanged();
}

void RosterController::handleJidRemoved(const Swift::JID &jid)
{
    std::cout << "RosterController::handleJidRemoved: " << jid.toString() << std::endl;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    while (it != rosterList_.end())
    {
        if ((*it)->getJid() == QString::fromStdString(jid.toBare().toString()))
        {
            it = rosterList_.erase(it);
            break;
        }
        else
        {
            ++it;
        }
    }

   emit rosterListChanged();
}

void RosterController::requestRosterFromClient(Swift::Client *client)
{
    client_->requestRoster();

    Swift::GetRosterRequest::ref rosterRequest = Swift::GetRosterRequest::create(client->getIQRouter());
    rosterRequest->onResponse.connect(bind(&RosterController::handleRosterReceived, this, _2));
    rosterRequest->send();
}

void RosterController::handleRosterReceived(Swift::ErrorPayload::ref error)
{
	if (error)
	{
		std::cerr << "Error receiving roster. Continuing anyway.";
	}
	else
	{
        Swift::VCardManager *vCardManager = client_->getVCardManager();
        vCardManager->onVCardChanged.connect(boost::bind(&RosterController::handleVCardChanged, this, _1, _2));

		Swift::XMPPRoster* roster = client_->getRoster();
		std::vector<Swift::XMPPRosterItem> rosterItems = roster->getItems();

		std::vector<Swift::XMPPRosterItem>::iterator it;

		for(it = rosterItems.begin(); it < rosterItems.end(); it++ )
		{
            vCardManager->requestVCard((*it).getJID());

            rosterList_.append(new RosterItem(QString::fromStdString((*it).getJID().toBare().toString()),
                                              QString::fromStdString((*it).getName()),
                                              (RosterItem::Subscription)(*it).getSubscription(), false, this));

			emit rosterListChanged();
		}
	}
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
    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();
    boost::shared_ptr<Swift::RosterPayload> payload(new Swift::RosterPayload);

    Swift::RosterItemPayload riPayload;
    riPayload.setJID(jid.toStdString());
    riPayload.setName(name.toStdString());
    payload->addItem(riPayload);

    Swift::IQRouter *iqRouter = client_->getIQRouter();
    iqRouter->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, Swift::JID(), msgId, payload));
}

void RosterController::removeContact(const QString& jid)
{
    Swift::IDGenerator idGenerator;
    std::string msgId = idGenerator.generateID();
    boost::shared_ptr<Swift::RosterPayload> payload(new Swift::RosterPayload);
    payload->addItem(Swift::RosterItemPayload(Swift::JID(jid.toStdString()), "", Swift::RosterItemPayload::Remove));
    Swift::IQRouter *iqRouter = client_->getIQRouter();
    iqRouter->sendIQ(Swift::IQ::createRequest(Swift::IQ::Set, Swift::JID(), msgId, payload));
}

QQmlListProperty<RosterItem> RosterController::getRosterList()
{
	return QQmlListProperty<RosterItem>(this, rosterList_);
}

void RosterController::addGroupAsContact(QString groupJid, QString groupName)
{
    rosterList_.append(new RosterItem(groupJid, groupName, RosterItem::SUBSCRIPTION_NONE, true, this));

    emit rosterListChanged();
}

void RosterController::removeGroupFromContacts(QString groupJid)
{
    QList<RosterItem*>::iterator it = rosterList_.begin();

    for (; it != rosterList_.end(); ++it)
    {
        if ((*it)->getJid() == groupJid)
        {
            it = rosterList_.erase(it);
            break;
        }
    }

    emit rosterListChanged();
}

bool RosterController::isGroup(QString const &jid)
{
    bool returnValue = true;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if (jid.compare((*it)->getJid()) == 0)
        {
            returnValue = false;
            break;
        }
    }

    return returnValue;
}

QString RosterController::getAvatarImagePathForJid(QString const &jid)
{
    QString returnValue = "";

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if (jid.compare((*it)->getJid()) == 0)
        {
            returnValue = (*it)->getImagePath();
            break;
        }
    }

    return returnValue;
}
