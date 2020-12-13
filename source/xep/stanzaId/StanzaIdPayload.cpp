#include "StanzaIdPayload.h"

StanzaIdPayload::StanzaIdPayload()
{
}

const std::string& StanzaIdPayload::getBy() const
{
    return by_;
}

const std::string& StanzaIdPayload::getId() const
{ 
    return id_;
}

void StanzaIdPayload::setBy(const std::string& by)
{
    this->by_ = by;
}

void StanzaIdPayload::setId(const std::string& id)
{
    this->id_ = id;
}
