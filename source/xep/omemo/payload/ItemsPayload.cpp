#include "ItemsPayload.h"

const std::string& ItemsPayload::getItemsPayload() const
{
    return itemsPayload_;
}

void ItemsPayload::setItemsPayload(const std::string& payload)
{
    itemsPayload_ = payload;
}

const std::string& ItemsPayload::getNode() const
{
    return node_;
}

void ItemsPayload::setNode(const std::string& node)
{
    node_ = node;
}
