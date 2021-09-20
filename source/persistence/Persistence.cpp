#include "Persistence.h"
#include "Database.h"
#include "MessageController.h"
#include "SessionController.h"
#include "GcmController.h"

#include <QDebug>

Persistence::Persistence(QObject *parent)
    : QObject(parent),
      db_(new Database(this)),
      messageController_(new MessageController(db_, this)),
      sessionController_(new SessionController(db_, this)),
      gcmController_(new GcmController(db_, this)),
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
        gcmController_->setup();

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

void Persistence::addMessage(QString const &id, QString const &jid, QString const &resource, QString const &message,
                             QString const &type, unsigned int direction, unsigned int security, qint64 timestamp)
{
    if (persistenceValid_)
    {
        if (messageController_->addMessage(id, jid, resource, message, type, direction, security, timestamp))
        {
            sessionController_->updateSession(jid, message);

            emit messageControllerChanged();
            emit sessionControllerChanged();
        }
    }
}

void Persistence::removeMessage(QString const &id, const QString &jid)
{
    if (persistenceValid_)
    {
        if (messageController_->removeMessage(id, jid))
        {
            //sessionController_->updateSession(jid, message);

            emit messageControllerChanged();
            //emit sessionControllerChanged();
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

void Persistence::markMessageAsUploadingAttachment(QString const &id)
{
    if (persistenceValid_)
    {
        messageController_->markMessageUploadingAttachment(id);
    }
}

void Persistence::markMessageAsSendFailed(QString const &id)
{
    if (persistenceValid_)
    {
        messageController_->markMessageSendFailed(id);
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

void Persistence::markGroupMessageReceivedByMember(const QString &msgId, const QString &groupChatMember)
{
    gcmController_->markGroupMessageReceivedByMember(msgId, groupChatMember);
    emit gcmControllerChanged();
}

void Persistence::markGroupMessageDisplayedByMember(const QString &msgId, const QString &groupChatMember)
{
    gcmController_->markGroupMessageDisplayedByMember(msgId, groupChatMember);
    emit gcmControllerChanged();
}


QPair<QString, int> Persistence::getNewestReceivedMessageIdAndStateOfJid(QString const &jid)
{
    return messageController_->getNewestReceivedMessageIdAndStateOfJid(jid);
}

QString Persistence::getResourceForMsgId(const QString& msgId)
{
    return messageController_->getRessourceForMsgId(msgId);
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

GcmController* Persistence::getGcmController()
{
    return gcmController_;
}

const QString Persistence::getResourcesOfNewestDisplayedMsgforJid(const QString& jid)
{
    return gcmController_->getResourcesOfNewestDisplayedMsgforJid(jid);
}

void Persistence::removeConversation(const QString& jid)
{
    getMessageController()->removeMessagesFromJid(jid);
    getSessionController()->removeSession(jid);
}