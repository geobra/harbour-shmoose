#include "XmppPingController.h"
#include "PingRequest.h"

#include <Swiften/Swiften.h>

#include <QTime>
#include <QDebug>

XmppPingController::XmppPingController() : client_(NULL)
{
}

void XmppPingController::setupWithClient(Swift::Client* client)
{
    client_ = client;
}

void XmppPingController::doPing()
{
    if (client_ != NULL)
    {
        PingRequest::ref pingRequest = PingRequest::create(Swift::JID(client_->getJID().getDomain()),
                                                           client_->getIQRouter());
        pingRequest->onResponse.connect(boost::bind(&XmppPingController::handlePingResponse, this, _1));
        pingRequest->send();
    }
}

void XmppPingController::handlePingResponse(const std::string response)
{
    qDebug() << QTime::currentTime().toString() << "handlePingResponse: " << QString::fromStdString(response);
}
