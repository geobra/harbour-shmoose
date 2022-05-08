#pragma once


#include <memory>
#include <typeinfo>

#include <boost/signals2.hpp>

#include <Swiften/Base/API.h>
#include <Swiften/Elements/ErrorPayload.h>
#include <Swiften/Elements/RawXMLPayload.h>
#include <Swiften/Elements/MAMFin.h>
#include <Swiften/Queries/Request.h>

class MamRequest : public Swift::Request {
public:
    typedef std::shared_ptr<MamRequest> ref;

    static ref create(Swift::IQ::Type type, const std::string& jid, const std::string& data, Swift::IQRouter* router) {
        return ref(new MamRequest(type, jid, data, router));
    }

    boost::signals2::signal<void (const std::string& jid, std::shared_ptr<Swift::MAMFin> mamFin, Swift::ErrorPayload::ref error) > onResponse;

private:
    MamRequest(Swift::IQ::Type type, const std::string& jid, const std::string& data, Swift::IQRouter* router) : 
        Swift::Request(type, Swift::JID(jid), std::make_shared<Swift::RawXMLPayload>(data), router) {
    }

    virtual void handleResponse(std::shared_ptr<Swift::Payload> payload, Swift::ErrorPayload::ref error) {
        onResponse(getReceiver().toString(), std::dynamic_pointer_cast<Swift::MAMFin>(payload), error);
    }
};
