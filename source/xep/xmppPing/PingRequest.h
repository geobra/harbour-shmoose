#ifndef PINGREQUEST_H
#define PINGREQUEST_H

#include <Swiften/Swiften.h>
#include <Swiften/Elements/RawXMLPayload.h>

class PingRequest : public Swift::Request
{
public:
    typedef std::shared_ptr<PingRequest> ref;

    static ref create(const Swift::JID& recipient, Swift::IQRouter* router)
    {
        return ref(new PingRequest(recipient, router));
    }

    boost::signals2::signal<void (const std::string&)> onResponse;

private:
    PingRequest(const Swift::JID& receiver, Swift::IQRouter* router) :
        Request(Swift::IQ::Type::Get, receiver, std::make_shared<Swift::RawXMLPayload>("<ping xmlns='urn:xmpp:ping'/>"), router)
    {
    }

    void handleResponse (std::shared_ptr< Swift::Payload > payload, std::shared_ptr< Swift::ErrorPayload > error)
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
