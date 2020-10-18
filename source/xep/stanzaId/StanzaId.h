#pragma once

#include <QObject>

#include <Swiften/Swiften.h>

// https://xmpp.org/extensions/xep-0359.html
// unique and stable IDs for messages, used to identify messages in xep-0313 (MAM)

class StanzaIdPayload : public Swift::Payload {
public:
    StanzaIdPayload() {}

    const std::string& getBy() const {
        return by;
    }

    const std::string& getId() const {
        return id;
    }

    void setBy(const std::string& by) {
        this->by = by;
    }

    void setId(const std::string& id) {
        this->id = id;
    }

private:
    std::string by;
    std::string id;
};

class StanzaIdPayloadParser : public Swift::GenericPayloadParser<StanzaIdPayload> {
public:
    StanzaIdPayloadParser() : currentDepth(0) {}

    void handleStartElement(const std::string& element, const std::string& /* ns */, const Swift::AttributeMap& attributes) {
        if (currentDepth == 0) {
            getPayloadInternal()->setBy(attributes.getAttribute("by"));
            getPayloadInternal()->setId(attributes.getAttribute("id"));
        }
        currentDepth++;
    }

    void handleEndElement(const std::string& /* element */, const std::string& /* ns */) {
        currentDepth--;
    }

    void handleCharacterData(const std::string& data) {
    }

private:
    int currentDepth;
    std::string by, id;
};

class StanzaIdPayloadParserFactory : public Swift::GenericPayloadParserFactory<StanzaIdPayloadParser> {
public:
    StanzaIdPayloadParserFactory() : GenericPayloadParserFactory<StanzaIdPayloadParser>("stanza-id", "urn:xmpp:sid:0") {}
};

class StanzaIdPayloadSerializer : public Swift::GenericPayloadSerializer<StanzaIdPayload> {
public:
    std::string serializePayload(std::shared_ptr<StanzaIdPayload> payload) const {
        Swift::XMLElement element("stanza-id", "urn:xmpp:sid:0");
        element.setAttribute("id", payload->getId());
        element.setAttribute("by", payload->getBy());
        return element.serialize();
    }
};

class StanzaId : public QObject
{
    Q_OBJECT
public:
    StanzaId(QObject *parent = 0);
    void setupWithClient(Swift::Client *client);

signals:

public slots:

private:
    StanzaIdPayloadParserFactory stanzaIdPayloadParserFactory;
    StanzaIdPayloadSerializer stanzaIdPayloadSerializer;
};
