#include "EncryptionPayloadSerializer.h"

std::string EncryptionPayloadSerializer::serializePayload(std::shared_ptr<EncryptionPayload> payload) const
{
    Swift::XMLElement element("encryption", "urn:xmpp:eme:0");

    element.setAttribute("namespace", payload->getNamespace());
    element.setAttribute("name", payload->getName());
    return element.serialize();
}
