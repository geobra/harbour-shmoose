#include "StanzaIdPayloadParserTester.h"

StanzaIdPayloadParserTester::StanzaIdPayloadParserTester()
{
    xmlParser_ = Swift::PlatformXMLParserFactory().createXMLParser(this);
    factories_.addFactory(new StanzaIdPayloadParserFactory());
}

bool StanzaIdPayloadParserTester::parse(const std::string& data)
{
    return xmlParser_->parse(data);
}

void StanzaIdPayloadParserTester::handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes)
{
    if (level_ == 0)
    {
        assert(!payloadParser_.get());
        Swift::PayloadParserFactory* payloadParserFactory = factories_.getPayloadParserFactory(element, ns, attributes);
        assert(payloadParserFactory);
        payloadParser_.reset(payloadParserFactory->createPayloadParser());
    }
    payloadParser_->handleStartElement(element, ns, attributes);
    level_++;
}

void StanzaIdPayloadParserTester::handleEndElement(const std::string& element, const std::string& ns)
{
    level_--;
    payloadParser_->handleEndElement(element, ns);
}

void StanzaIdPayloadParserTester::handleCharacterData(const std::string& data)
{
    payloadParser_->handleCharacterData(data);
}

std::shared_ptr<StanzaIdPayload> StanzaIdPayloadParserTester::getPayload() const
{
    return std::dynamic_pointer_cast<StanzaIdPayload>(payloadParser_->getPayload());
}
