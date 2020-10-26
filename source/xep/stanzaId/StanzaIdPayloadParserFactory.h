#pragma once

#include "StanzaIdPayloadParser.h"

// Swiften payload parser factory extension for xep-0359
// (https://swift.im/swiften/guide/#Section-Extending)
class StanzaIdPayloadParserFactory : public Swift::GenericPayloadParserFactory<StanzaIdPayloadParser>
{
public:
    StanzaIdPayloadParserFactory();
};
