#include "ItemsPayloadSerializer.h"

std::string ItemsPayloadSerializer::serializePayload(std::shared_ptr<ItemsPayload> payload) const
{
    return payload->getItemsPayload();
}
