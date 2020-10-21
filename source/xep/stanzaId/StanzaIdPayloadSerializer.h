#pragma once

#include "StanzaIdPayload.h"

// Swiften payload serializer extension for xep-0359
// (https://swift.im/swiften/guide/#Section-Extending)
class StanzaIdPayloadSerializer : public Swift::GenericPayloadSerializer<StanzaIdPayload>
{
public:
    std::string serializePayload(std::shared_ptr<StanzaIdPayload> payload) const;
};
