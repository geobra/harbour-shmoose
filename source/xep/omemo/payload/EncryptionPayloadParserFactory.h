#pragma once

#include "EncryptionPayloadParser.h"

class EncryptionPayloadParserFactory : public Swift::GenericPayloadParserFactory<EncryptionPayloadParser>
{
public:
    EncryptionPayloadParserFactory();
};
