#include "XmppPingController.h"
#include "PingRequest.h"

#include <Swiften/Swiften.h>
#include <QDebug>

XmppPingController::XmppPingController() : client_(NULL)
{
}

void XmppPingController::setClient(Swift::Client* client)
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
		qDebug() << "handlePingResponse: " << QString::fromStdString(response);
}
