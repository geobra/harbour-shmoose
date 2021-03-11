#pragma once

#include "EncryptionPayload.h"

class EncryptionPayloadParser : public Swift::GenericPayloadParser<EncryptionPayload>
{
public:
    EncryptionPayloadParser() {};

    void handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes);
    void handleEndElement(const std::string& element, const std::string& /* ns */);
    void handleCharacterData(const std::string& data);

private:
    int level_{0};
};
