#include "StanzaIdPayloadParserFactory.h"

StanzaIdPayloadParserFactory::StanzaIdPayloadParserFactory() : GenericPayloadParserFactory<StanzaIdPayloadParser>("stanza-id", "urn:xmpp:sid:0")
{
}
