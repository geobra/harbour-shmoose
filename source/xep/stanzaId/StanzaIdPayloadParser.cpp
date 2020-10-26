#include "StanzaIdPayloadParser.h"

StanzaIdPayloadParser::StanzaIdPayloadParser()
{
}

void StanzaIdPayloadParser::handleStartElement(const std::string& /* element */, const std::string& /* ns */, const Swift::AttributeMap& attributes)
{
    if (level_ == 0)
    {
        getPayloadInternal()->setBy(attributes.getAttribute("by"));
        getPayloadInternal()->setId(attributes.getAttribute("id"));
    }
    level_++;
}

void StanzaIdPayloadParser::handleEndElement(const std::string& /* element */, const std::string& /* ns */)
{
    level_--;
}

void StanzaIdPayloadParser::handleCharacterData(const std::string& /* data */)
{
}
