#include "RosterContoller.h"

#include <QQmlContext>
#include <QDebug>

RosterController::RosterController(QObject *parent) : QObject(parent), client_(NULL), rosterList_()
{
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
		//std::cout << "handleRosterReceived!!!" << std::endl;
		Swift::XMPPRoster* roster = client_->getRoster();
		std::vector<Swift::XMPPRosterItem> rosterItems = roster->getItems();

		std::vector<Swift::XMPPRosterItem>::iterator it;
		//std::cout << "size: " << rosterItems.size() << std::endl;

		for(it = rosterItems.begin(); it < rosterItems.end(); it++ )
		{
#if 0
            std::cout << "jid: " << (*it).getJID().toString() <<
						 ", Name: " << (*it).getName() <<
						 ", Subscription: " << (*it).getSubscription() << std::endl;
#endif

            rosterList_.append(new RosterItem(QString::fromStdString((*it).getJID().toBare().toString()),
                                              QString::fromStdString((*it).getName()),
                                              (RosterItem::Subscription)(*it).getSubscription(), false, this));

			emit rosterListChanged();
		}
	}
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
    bool returnValue = false;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if ((*it)->getJid() == jid)
        {
            returnValue = (*it)->isGroup();
            break;
        }
    }

    return returnValue;
}
