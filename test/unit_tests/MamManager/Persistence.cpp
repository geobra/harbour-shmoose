#include "Persistence.h"

Persistence::Persistence()
{

}

void Persistence::addMessage(const QString &id, QString const &jid, QString const &resource, QString const &message, const QString &type, unsigned int direction, qint64 timestamp)
{
    id_ = id;
    jid_ = jid;
    resource_ = resource;
    message_ = message;
    type_ = type;
    direction_ = direction;
    timestamp_ = timestamp;
}

void Persistence::markGroupMessageReceivedByMember(const QString &msgId, const QString &resource)
{
    id_ = msgId;
    resource_ = resource;
}

void Persistence::markGroupMessageDisplayedByMember(const QString &msgId, const QString &resource)
{
    id_ = msgId;
    resource_ = resource;
}

void Persistence::markMessageAsReceivedById(const QString &msgId)
{
    id_ = msgId;
}

void Persistence::markMessageAsDisplayedId(const QString &msgId)
{
    id_ = msgId;
}

void Persistence::clear()
{
    id_ = "";
    jid_ = "";
    resource_ = "";
    message_ = "";
    type_ = "";
    direction_ = 255;
    timestamp_ = 0;
}

