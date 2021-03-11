#include "EncryptionPayloadParserFactory.h"

// FIXME use correct ns!
EncryptionPayloadParserFactory::EncryptionPayloadParserFactory() : GenericPayloadParserFactory<EncryptionPayloadParser>("encryption", "urn:xmpp:eme:0")
{
}
