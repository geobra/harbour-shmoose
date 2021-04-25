#include "EncryptionPayloadParserFactory.h"

EncryptionPayloadParserFactory::EncryptionPayloadParserFactory() : GenericPayloadParserFactory<EncryptionPayloadParser>("encryption", "urn:xmpp:eme:0")
{
}
