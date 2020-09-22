#include "ConnectionHandler.h"
#include "ReConnectionHandler.h"
#include "IpHeartBeatWatcher.h"
#include "XmppPingController.h"

#include <boost/bind.hpp>

#include <QTime>
#include <QDebug>

ConnectionHandler::ConnectionHandler(QObject *parent) : QObject(parent),
    connected_(false), initialConnectionSuccessfull_(false), hasInetConnection_(false), appIsActive_(false),
    reConnectionHandler_(new ReConnectionHandler(30000, this)),
    ipHeartBeatWatcher_(new IpHeartBeatWatcher(this)),
    xmppPingController_(new XmppPingController())
{
    connect(reConnectionHandler_, SIGNAL(canTryToReconnect()), this, SLOT(tryReconnect()));

#ifdef SFOS
    connect(ipHeartBeatWatcher_, SIGNAL(triggered()), this, SLOT(tryStablishServerConnection()));
    connect(ipHeartBeatWatcher_, SIGNAL(finished()), ipHeartBeatWatcher_, SLOT(deleteLater()));
#else
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(tryStablishServerConnection()));
    timer->start(60000 * 20); // 20 minutes
#endif

    ipHeartBeatWatcher_->start();
}

ConnectionHandler::~ConnectionHandler()
{
    delete xmppPingController_;

    ipHeartBeatWatcher_->stopWatching();
#ifdef SFOS
    ipHeartBeatWatcher_->terminate();
#endif
}

void ConnectionHandler::setupWithClient(Swift::Client* client)
{
    if (client != nullptr)
    {
        client_ = client;

        client_->onConnected.connect(boost::bind(&ConnectionHandler::handleConnected, this));
        client_->onDisconnected.connect(boost::bind(&ConnectionHandler::handleDisconnected, this, _1));

        xmppPingController_->setupWithClient(client_);
    }
}

void ConnectionHandler::handleConnected()
{
    qDebug() << QTime::currentTime().toString() << "ConnectionHandler::handleConnected";

    connected_ = true;
    emit connectionStateChanged();

    // only on a first connection. skip this on a reconnect event.
    if (initialConnectionSuccessfull_ == false)
    {
        initialConnectionSuccessfull_ = true;

        reConnectionHandler_->setActivated();

        emit signalInitialConnectionEstablished();
    }

    client_->sendPresence(Swift::Presence::create("Send me a message"));    // FIXME presence handler
    // message carbons request must be made each time a connection is established
    enableMessageCarbons();
}

void ConnectionHandler::enableMessageCarbons() {
    auto enableCarbonsRequest = Swift::EnableCarbonsRequest::create(client_->getIQRouter());
    enableCarbonsRequest->onResponse.connect(boost::bind(&ConnectionHandler::handleCarbonsReply, this, _1, _2));
    enableCarbonsRequest->send();
}

void ConnectionHandler::handleCarbonsReply(Swift::Payload::ref, Swift::ErrorPayload::ref error){
    if (error) {
    	qDebug() << "Failed to enable carbons";
    } else {
    	qDebug() << "Successfully enabled carbons";
    }
}

void ConnectionHandler::handleDisconnected(const boost::optional<Swift::ClientError>& error)
{
    qDebug() << QTime::currentTime().toString() << "ConnectionHandler::handleDISconnected";

    connected_ = false;
    emit connectionStateChanged();

    if (error)
    {
        Swift::ClientError clientError = *error;
        Swift::ClientError::Type type = clientError.getType();
        qDebug() << "disconnet error: " << type;

        // trigger the reConnectionHandler to get back online if inet is available
        if (initialConnectionSuccessfull_)
        {
#ifndef TRAVIS
            // dont do a automatic reconnect attempmt on TRAVIS during testing
            reConnectionHandler_->isConnected(hasInetConnection_);
#endif
        }
    }
    else
    {
        qDebug() << "disconnect without error";
    }
}

void ConnectionHandler::tryStablishServerConnection()
{
    qDebug() << QTime::currentTime().toString() << " ConnectionHandler::tryStablishServerConnection.";

    if (client_ != nullptr)
    {
         qDebug() << "  clientActive: " << client_->isActive() ;

        if (hasInetConnection_ == true
                && client_->isActive() == true
#ifdef SFOS
                && appIsActive_ == false /* SFOS specific. Connection wont be droped if app is in use */
#endif
                )
        {
            xmppPingController_->doPing();
        }
        else
        {
            // test to trigger a reconnect if not connected
            reConnectionHandler_->isConnected(hasInetConnection_);
        }
    }
}

bool ConnectionHandler::isConnected()
{
    return connected_;
}

void ConnectionHandler::tryReconnect()
{
    qDebug() << QTime::currentTime().toString() << "ConnectionHandler::tryReconnect";

    if (initialConnectionSuccessfull_ == true && hasInetConnection_ == true)
    {
        // try to disconnect the old session from before network disturbtion
        client_->disconnect();

        // try new connect
        client_->connect();
    }
}

void ConnectionHandler::setHasInetConnection(bool connected)
{
    hasInetConnection_ = connected;
    reConnectionHandler_->isConnected(connected);

    // emit disconnect signal on every change of inet connection.
    // signalHasInetConnection(true) will be send in ConnectionHandler::handleConnected()
    emit signalHasInetConnection(false);
}

void ConnectionHandler::slotAppGetsActive(bool active)
{
    appIsActive_ = active;
}
