#pragma once

#include "ItemsPayload.h"

class ItemsPayloadSerializer : public Swift::GenericPayloadSerializer<ItemsPayload>
{
public:
    std::string serializePayload(std::shared_ptr<ItemsPayload> payload) const;
};
