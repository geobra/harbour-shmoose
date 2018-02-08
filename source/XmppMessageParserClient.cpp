#include "XmppMessageParserClient.h"

#include <QString>

XMPPMessageParserClient::XMPPMessageParserClient() : Swift::XMPPParserClient(), msg_(new Swift::Message())
{
}

XMPPMessageParserClient::~XMPPMessageParserClient()
{
    delete msg_;
}

Swift::Message* XMPPMessageParserClient::getMessagePtr()
{
    return msg_;
}

void XMPPMessageParserClient::handleStreamStart(const Swift::ProtocolHeader&)
{
    //std::cout << "-> Stream start" << std::endl;
}

void XMPPMessageParserClient::handleElement(boost::shared_ptr<Swift::ToplevelElement> element)
{
    std::string elementName = typeid(*element.get()).name();
    if (elementName.find("Message") != std::string::npos)
    {
        Swift::Message* pMsg = static_cast<Swift::Message*>(&(*element));
        if (pMsg != nullptr)
        {
            // copy the object
            *msg_ = *pMsg;
        }
    }
    else
    {
        // was not a message
        msg_->setBody("");
    }
}

void XMPPMessageParserClient::handleStreamEnd()
{
    //std::cout << "-> Stream end" << std::endl;
}
