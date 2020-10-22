#pragma once

#include "StanzaIdPayload.h"

// Swiften payload parser extension for xep-0359
// (https://swift.im/swiften/guide/#Section-Extending)
class StanzaIdPayloadParser : public Swift::GenericPayloadParser<StanzaIdPayload>
{
public:
    StanzaIdPayloadParser();

    void handleStartElement(const std::string& /* element */, const std::string& /* ns */, const Swift::AttributeMap& attributes);
    void handleEndElement(const std::string& /* element */, const std::string& /* ns */);
    void handleCharacterData(const std::string& /* data */);

private:
    int level_{0};
};
