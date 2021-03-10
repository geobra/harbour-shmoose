#include "ItemsPayloadParserFactory.h"

ItemsPayloadParserFactory::ItemsPayloadParserFactory() : GenericPayloadParserFactory<ItemsPayloadParser>("event", "http://jabber.org/protocol/pubsub#event")
{

}
