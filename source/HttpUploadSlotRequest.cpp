#include "HttpUploadSlotRequest.h"

HttpUploadSlotRequest::HttpUploadSlotRequest(Swift::Client* client)
    : Swift::Request(Swift::IQ::Type::Get, Swift::JID(client->getJID().getDomain()), client->getIQRouter())
{
#if 0
    Swift::Payload::ref *payload = new Swift::Payload
    this->setPayload(payload);
#endif
}
