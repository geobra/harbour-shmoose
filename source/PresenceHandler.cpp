#include "PresenceHandler.h"
#include "RosterContoller.h"

PresenceHandler::PresenceHandler(RosterController *rosterController) : QObject(rosterController),
    client_(NULL), rosterController_(rosterController)
{

}

void PresenceHandler::setupWithClient(Swift::Client* client)
{
    if (client != NULL)
    {
        client_ = client;

        client_->onPresenceReceived.connect(boost::bind(&PresenceHandler::handlePresenceReceived, this, _1));
        client_->onPresenceChange.connect(boost::bind(&PresenceHandler::handlePresenceChanged, this, _1));
    }
}

void PresenceHandler::handlePresenceReceived(Swift::Presence::ref presence)
{
    // Automatically approve subscription requests
    // FIXME show to user and let user decide
    if (presence->getType() == Swift::Presence::Subscribe)
    {
        // answer subscription request
        Swift::Presence::ref subscriptionRequestResponse = Swift::Presence::create();
        subscriptionRequestResponse->setTo(presence->getFrom());
        subscriptionRequestResponse->setFrom(client_->getJID());
        subscriptionRequestResponse->setType(Swift::Presence::Subscribed);
        client_->sendPresence(subscriptionRequestResponse);

        // request subscription
        Swift::Presence::ref subscriptionRequest = Swift::Presence::create();
        subscriptionRequest->setTo(presence->getFrom());
        subscriptionRequest->setFrom(client_->getJID());
        subscriptionRequest->setType(Swift::Presence::Subscribe);
        client_->sendPresence(subscriptionRequest);
    }
}

void PresenceHandler::handlePresenceChanged(Swift::Presence::ref presence)
{
    //qDebug() << "handlePresenceChanged: type: " << presence->getType() << ", jid: " << QString::fromStdString(presence->getFrom());

    Swift::JID jid = presence->getFrom();
    QString status = "";

    if (presence->getType() == Swift::Presence::Available)
    {
        std::vector<boost::shared_ptr<Swift::Status> > availabilityPayloads = presence->getPayloads<Swift::Status>();

        for (std::vector<boost::shared_ptr<Swift::Status>>::iterator it = availabilityPayloads.begin() ; it != availabilityPayloads.end(); ++it)
        {
            status = QString::fromStdString((*it)->getText());
            break;
        }
    }

    if (jid.isValid())
    {
        RosterItem::Availability availability = RosterItem::AVAILABILITY_ONLINE;

        if (presence->getType() == Swift::Presence::Unavailable
                || presence->getType() == Swift::Presence::Error
                || presence->getType() == Swift::Presence::Probe
                )
        {
            availability = RosterItem::AVAILABILITY_OFFLINE;
        }

        rosterController_->handleUpdateFromPresence(jid, status, availability);
    }
}
