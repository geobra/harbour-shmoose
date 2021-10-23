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

class BundleDeviceListRequest : public Swift::Request {
public:
    typedef std::shared_ptr<BundleDeviceListRequest> ref;

    static ref create(Swift::IQ::Type type, const std::string& data, Swift::IQRouter* router) {
        return ref(new BundleDeviceListRequest(type, data, router));
    }

    boost::signals2::signal<void (const std::string&)> onResponse;

private:
    BundleDeviceListRequest(Swift::IQ::Type type, const std::string& data, Swift::IQRouter* router) : Swift::Request(type, "", std::make_shared<Swift::RawXMLPayload>(data), router) {
    }

    virtual void handleResponse(std::shared_ptr<Swift::Payload> payload, Swift::ErrorPayload::ref error) {
        if (error) {
            onResponse(Swift::ErrorSerializer(&serializers).serializePayload(error));
        }
        else {
            if(payload)
            {
                Swift::PayloadSerializer* serializer = serializers.getPayloadSerializer(payload);
                assert(serializer);
                onResponse(serializer->serialize(payload));
            }
        }
    }

private:
    Swift::FullPayloadSerializerCollection serializers;
};
