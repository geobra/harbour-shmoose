#include "StanzaIdPayloadSerializer.h"
 
std::string StanzaIdPayloadSerializer::serializePayload(std::shared_ptr<StanzaIdPayload> payload) const
{
    Swift::XMLElement element("stanza-id", "urn:xmpp:sid:0");
    element.setAttribute("id", payload->getId());
    element.setAttribute("by", payload->getBy());
    return element.serialize();
}
