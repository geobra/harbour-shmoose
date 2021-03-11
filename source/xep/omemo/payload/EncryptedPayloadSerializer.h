#pragma once

#include "EncryptedPayload.h"

class EncryptedPayloadSerializer : public Swift::GenericPayloadSerializer<EncryptedPayload>
{
public:
    std::string serializePayload(std::shared_ptr<EncryptedPayload> payload) const;
};
