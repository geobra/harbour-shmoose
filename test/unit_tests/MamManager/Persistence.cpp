#include "Persistence.h"

Persistence::Persistence()
{

}

void Persistence::addMessage(const QString &id, QString const &jid, QString const &resource, QString const &message, const QString &type, unsigned int direction)
{
    id_ = id;
    jid_ = jid;
    resource_ = resource;
    message_ = message;
    type_ = type;
    direction_ = direction;
}

void Persistence::clear()
{
    id_ = "";
    jid_ = "";
    resource_ = "";
    message_ = "";
    type_ = "";
    direction_ = 255;
}

