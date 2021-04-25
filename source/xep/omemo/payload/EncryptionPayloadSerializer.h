#pragma once

#include "EncryptionPayload.h"

class EncryptionPayloadSerializer : public Swift::GenericPayloadSerializer<EncryptionPayload>
{
public:
    std::string serializePayload(std::shared_ptr<EncryptionPayload> payload) const;
};
