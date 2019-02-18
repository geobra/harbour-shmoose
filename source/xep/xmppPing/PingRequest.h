#ifndef PINGREQUEST_H
#define PINGREQUEST_H

#include <boost/smart_ptr/make_shared.hpp>
#include <Swiften/Swiften.h>
#include <Swiften/Elements/RawXMLPayload.h>

class PingRequest : public Swift::Request
{
public:
    typedef boost::shared_ptr<PingRequest> ref;

    static ref create(const Swift::JID& recipient, Swift::IQRouter* router)
    {
        return ref(new PingRequest(recipient, router));
    }

    boost::signal<void (const std::string&)> onResponse;

private:
    PingRequest(const Swift::JID& receiver, Swift::IQRouter* router) :
        Request(Swift::IQ::Type::Get, receiver, boost::make_shared<Swift::RawXMLPayload>("<ping xmlns='urn:xmpp:ping'/>"), router)
    {
    }

    void handleResponse (boost::shared_ptr< Swift::Payload > payload, boost::shared_ptr< Swift::ErrorPayload > error)
    {
        if (error)
        {
            onResponse(Swift::ErrorSerializer(&serializers).serializePayload(error));
        }
        else
        {
            (void) payload;
            onResponse("pong"); // empty payload on success ping answer (pong)
        }
    }

    Swift::FullPayloadSerializerCollection serializers;
};

#endif // PINGREQUEST_H
