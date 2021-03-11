#include "EncryptedPayloadSerializer.h"

std::string EncryptedPayloadSerializer::serializePayload(std::shared_ptr<EncryptedPayload> payload) const
{
    // FIXME use correct ns
    Swift::XMLElement element("encryption", "urn:xmpp:eme:0");

    element.setAttribute("namespace", payload->getNamespace());
    element.setAttribute("name", payload->getName());
    return element.serialize();
}
