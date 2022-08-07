#ifndef CLIENT_H
#define CLIENT_H

#include <Swiften/Swiften.h>
#include <Swiften/Client/Client.h>
#include <Swiften/Client/Storages.h>
#include <Swiften/JID/JID.h>
#include <Swiften/Base/SafeString.h>
#include <Swiften/Network/NetworkFactories.h>

#include <Swiften/Parser/GenericPayloadParserFactory2.h>
#include <Swiften/Parser/PayloadParsers/MAMResultParser.h>
#include <Swiften/Parser/PayloadParsers/MAMFinParser.h>

class  Client : public Swift::Client {
    public:
        Client(const Swift::JID& jid, const Swift::SafeString& password, Swift::NetworkFactories* networkFactories, Swift::Storages* storages = nullptr);


    private:
        Swift::GenericPayloadParserFactory2<Swift::MAMResultParser> mamResultParser;
        Swift::GenericPayloadParserFactory<Swift::MAMFinParser> mamFinParser;
};
#endif
