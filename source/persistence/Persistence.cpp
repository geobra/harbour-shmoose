#include "Persistence.h"
#include "Database.h"
#include "MessageController.h"
#include "SessionController.h"

Persistence::Persistence(QObject *parent) : QObject(parent), persistenceValid_(true)
{
	db_ = new Database(this);
	if (! db_->isValid())
	{
		persistenceValid_ = false;
	}
	else
	{
		messageController_ = new MessageController(db_, this);
		sessionController_ = new SessionController(db_, this);
	}
}

Persistence::~Persistence()
{
	// db_ has this as parent and gets free'd implicit from this;
}

void Persistence::addMessage(QString const &id, QString const &jid, QString const &message, QString const &type, unsigned int direction)
{
    messageController_->addMessage(id, jid, message, type, direction);
	sessionController_->updateSession(jid, message);

	emit messageControllerChanged();
	emit sessionControllerChanged();
}

void Persistence::markMessageAsReceivedById(QString const &id)
{
	messageController_->markMessageReceived(id);
}

void Persistence::setCurrentChatPartner(QString const &jid)
{
	messageController_->setFilterOnJid(jid);
	sessionController_->setCurrentChatPartner(jid);
	sessionController_->updateNumberOfUnreadMessages(jid, 0);
}

bool Persistence::isValid()
{
	return persistenceValid_;
}

MessageController* Persistence::getMessageController()
{
	return messageController_;
}

SessionController* Persistence::getSessionController()
{
	return sessionController_;
}
