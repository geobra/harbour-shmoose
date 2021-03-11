#include "EncryptedPayloadParserFactory.h"

// FIXME use correct ns!
EncryptedPayloadParserFactory::EncryptedPayloadParserFactory() : GenericPayloadParserFactory<EncryptedPayloadParser>("encryption", "urn:xmpp:eme:0")
{
}
