#pragma once


#include <memory>
#include <typeinfo>

#include <boost/signals2.hpp>

#include <Swiften/Base/API.h>
#include <Swiften/Elements/ErrorPayload.h>
#include <Swiften/Elements/RawXMLPayload.h>
#include <Swiften/Elements/MAMFin.h>
#include <Swiften/Elements/DiscoInfo.h>
#include <Swiften/Queries/Request.h>

class MucRequest : public Swift::Request {
public:
    typedef std::shared_ptr<MucRequest> ref;

    static ref create(const std::string& jid, Swift::IQRouter* router) {
        return ref(new MucRequest(jid, router));
    }

    boost::signals2::signal<void (const std::string& jid, std::shared_ptr<Swift::DiscoInfo> discoInfo, Swift::ErrorPayload::ref error) > onResponse;

private:
    MucRequest(const std::string& jid, Swift::IQRouter* router) : 
        Swift::Request(Swift::IQ::Get, Swift::JID(jid), std::make_shared<Swift::DiscoInfo>(), router) {
    }

    virtual void handleResponse(std::shared_ptr<Swift::Payload> payload, Swift::ErrorPayload::ref error) {
        onResponse(getReceiver().toString(), std::dynamic_pointer_cast<Swift::DiscoInfo>(payload), error);
    }
};
