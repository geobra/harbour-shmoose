#pragma once

#include <Swiften/Elements/Payload.h>
#include <Swiften/Parser/PayloadParser.h>
#include <Swiften/Parser/PayloadParserFactoryCollection.h>
#include <Swiften/Parser/PlatformXMLParserFactory.h>
#include <Swiften/Parser/XMLParser.h>
#include <Swiften/Parser/XMLParserClient.h>

#include "StanzaIdPayloadParserFactory.h"

class StanzaIdPayloadParserTester : public Swift::XMLParserClient
{
public:
    StanzaIdPayloadParserTester();

    bool parse(const std::string& data);

    virtual void handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes);
    virtual void handleEndElement(const std::string& element, const std::string& ns);
    virtual void handleCharacterData(const std::string& data);

    std::shared_ptr<StanzaIdPayload> getPayload() const;

private:
    std::unique_ptr<Swift::XMLParser> xmlParser_;
    Swift::PayloadParserFactoryCollection factories_;
    std::shared_ptr<Swift::PayloadParser> payloadParser_;
    int level_{0};
};
