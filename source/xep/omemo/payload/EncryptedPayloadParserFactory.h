#pragma once

#include "EncryptedPayloadParser.h"

class EncryptedPayloadParserFactory : public Swift::GenericPayloadParserFactory<EncryptedPayloadParser>
{
public:
    EncryptedPayloadParserFactory();
};
