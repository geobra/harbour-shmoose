#ifndef XMPPMESSAGEPARSERCLIENT_H
#define XMPPMESSAGEPARSERCLIENT_H

#include <Swiften/Swiften.h>

class XMPPMessageParserClient : public Swift::XMPPParserClient
{
public:
    XMPPMessageParserClient();
    ~XMPPMessageParserClient();

    Swift::Message *getMessagePtr();

private:
    virtual void handleStreamStart(const Swift::ProtocolHeader&);
    virtual void handleElement(std::shared_ptr<Swift::ToplevelElement> element);
    virtual void handleStreamEnd();

    Swift::Message* msg_;
};

#endif // XMPPMESSAGEPARSERCLIENT_H
