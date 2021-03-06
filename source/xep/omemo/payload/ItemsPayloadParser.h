#pragma once

#include "ItemsPayload.h"

class ItemsPayloadParser : public Swift::GenericPayloadParser<ItemsPayload>
{
public:
    ItemsPayloadParser();

    void handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes);
    void handleEndElement(const std::string& element, const std::string& /* ns */);
    void handleCharacterData(const std::string& data);

private:
    int level_{0};
    bool collectStanzas{false};
    std::string itemsStanza{};
};
