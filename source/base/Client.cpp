#include "Client.h"



Client::Client(const Swift::JID& jid, const Swift::SafeString& password, Swift::NetworkFactories* networkFactories, Swift::Storages* storages) : 
    Swift::Client(jid, password, networkFactories, storages),
    mamResultParser("result", "urn:xmpp:mam:2", getPayloadParserFactories())
{
    addPayloadParserFactory(&mamResultParser);
}