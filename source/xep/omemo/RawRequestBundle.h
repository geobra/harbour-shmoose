#pragma once

#include "LurchTypes.h"

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


class RawRequestBundle : public Swift::Request {
public:
    typedef std::shared_ptr<RawRequestBundle> ref;

    static ref create(Swift::IQ::Type type, const Swift::JID& recipient, const std::string& data, Swift::IQRouter* router, const std::string& bundleId, lurch_queued_msg* qMsg) {
        return ref(new RawRequestBundle(type, recipient, data, router, bundleId, qMsg));
    }

    boost::signals2::signal<void (const Swift::JID&, const std::string&, lurch_queued_msg*, const std::string&)> onResponse;

private:
    RawRequestBundle(Swift::IQ::Type type, const Swift::JID& receiver, const std::string& data, Swift::IQRouter* router, const std::string& bundleId, lurch_queued_msg* qMsg) :
        Swift::Request(type, receiver, std::make_shared<Swift::RawXMLPayload>(data), router), receiver_(receiver), bundleId_(bundleId), qMsg_(qMsg) {
    }
    Swift::JID receiver_{};
    std::string bundleId_{};
    lurch_queued_msg* qMsg_{nullptr};

    virtual void handleResponse(std::shared_ptr<Swift::Payload> payload, Swift::ErrorPayload::ref error) {
        if (error) {
            onResponse(receiver_, bundleId_, qMsg_, Swift::ErrorSerializer(&serializers).serializePayload(error));
        }
        else {
            assert(payload);
            Swift::PayloadSerializer* serializer = serializers.getPayloadSerializer(payload);
            assert(serializer);
            onResponse(receiver_, bundleId_, qMsg_, serializer->serialize(payload));
        }
    }

private:
    Swift::FullPayloadSerializerCollection serializers;
};

