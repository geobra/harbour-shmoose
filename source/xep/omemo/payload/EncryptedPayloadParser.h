#pragma once

#include "EncryptedPayload.h"

class EncryptedPayloadParser : public Swift::GenericPayloadParser<EncryptedPayload>
{
public:
    EncryptedPayloadParser() {};

    void handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes);
    void handleEndElement(const std::string& element, const std::string& /* ns */);
    void handleCharacterData(const std::string& data);

private:
    int level_{0};
};
