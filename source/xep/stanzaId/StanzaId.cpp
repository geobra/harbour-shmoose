#include "StanzaId.h"

StanzaId::StanzaId(QObject *parent) : QObject(parent)
{
}

void StanzaId::setupWithClient(Swift::Client *client)
{
    client->addPayloadParserFactory(&stanzaIdPayloadParserFactory_);
    client->addPayloadSerializer(&stanzaIdPayloadSerializer_);
}
