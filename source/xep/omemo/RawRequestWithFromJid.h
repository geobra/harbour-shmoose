#pragma once

#include <memory>
#include <typeinfo>

#include <boost/signals2.hpp>

#include <Swiften/Base/API.h>
#include <Swiften/Elements/ErrorPayload.h>
#include <Swiften/Elements/RawXMLPayload.h>
#include <Swiften/Queries/Request.h>
#include <Swiften/Serializer/PayloadSerializer.h>
#include <Swiften/Serializer/PayloadSerializers/ErrorSerializer.h>
#include <Swiften/Serializer/PayloadSerializers/FullPayloadSerializerCollection.h>


class RawRequestWithFromJid : public Swift::Request {
public:
    typedef std::shared_ptr<RawRequestWithFromJid> ref;

    static ref create(Swift::IQ::Type type, const Swift::JID& recipient, const std::string& data, Swift::IQRouter* router) {
        return ref(new RawRequestWithFromJid(type, recipient, data, router));
    }

    boost::signals2::signal<void (const Swift::JID&, const std::string&)> onResponse;

private:
    RawRequestWithFromJid(Swift::IQ::Type type, const Swift::JID& receiver, const std::string& data, Swift::IQRouter* router) : Swift::Request(type, receiver, std::make_shared<Swift::RawXMLPayload>(data), router) {
        receiver_ = receiver;
    }
    Swift::JID receiver_;

    virtual void handleResponse(std::shared_ptr<Swift::Payload> payload, Swift::ErrorPayload::ref error) {
        if (error) {
            onResponse(receiver_, Swift::ErrorSerializer(&serializers).serializePayload(error));
        }
        else {
            assert(payload);
            Swift::PayloadSerializer* serializer = serializers.getPayloadSerializer(payload);
            assert(serializer);
            onResponse(receiver_, serializer->serialize(payload));
        }
    }

private:
    Swift::FullPayloadSerializerCollection serializers;
};

