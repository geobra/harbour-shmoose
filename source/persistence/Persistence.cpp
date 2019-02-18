#include "Persistence.h"
#include "Database.h"
#include "MessageController.h"
#include "SessionController.h"

#include <QDebug>

Persistence::Persistence(QObject *parent)
    : QObject(parent),
      db_(new Database(this)),
      messageController_(new MessageController(db_, this)),
      sessionController_(new SessionController(db_, this)),
      currentChatPartner_(""),
      persistenceValid_(false)
{
}

Persistence::~Persistence()
{
    // db_ has this as parent and gets free'd implicit from this;
}

void Persistence::openDatabaseForJid(QString const &jid)
{
    if (db_->open(jid))
    {
        messageController_->setup();
        sessionController_->setup();

        persistenceValid_ = true;
    }
    else
    {
        qDebug() << "failed to open db for " << jid;
    }
}

QString Persistence::getCurrentChatPartner()
{
    return currentChatPartner_;
}

void Persistence::addMessage(bool isGroupMessage, QString const &id, QString const &jid, QString const &resource, QString const &message, QString const &type, unsigned int direction)
{
    if (persistenceValid_)
    {
        if (messageController_->addMessage(isGroupMessage, id, jid, resource, message, type, direction))
        {
            sessionController_->updateSession(jid, message);

            emit messageControllerChanged();
            emit sessionControllerChanged();
        }
    }
}

void Persistence::markMessageDisplayedConfirmedId(QString const &id)
{
    if (persistenceValid_)
    {
        messageController_->markMessageDisplayedConfirmed(id);
    }
}

void Persistence::markMessageAsDisplayedId(QString const &id)
{
    if (persistenceValid_)
    {
        messageController_->markMessageDisplayed(id);
    }
}

void Persistence::markMessageAsReceivedById(QString const &id)
{
    if (persistenceValid_)
    {
        messageController_->markMessageReceived(id);
    }
}

void Persistence::markMessageAsSentById(QString const &id)
{
    if (persistenceValid_)
    {
        messageController_->markMessageSent(id);
    }
}

void Persistence::setCurrentChatPartner(QString const &jid)
{
    currentChatPartner_ = jid;

    if (persistenceValid_)
    {
        messageController_->setFilterOnJid(jid);
        sessionController_->setCurrentChatPartner(jid);
        sessionController_->updateNumberOfUnreadMessages(jid, 0);
    }
}

QPair<QString, int> Persistence::getNewestReceivedMessageIdAndStateOfJid(QString const &jid)
{
    return messageController_->getNewestReceivedMessageIdAndStateOfJid(jid);
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
