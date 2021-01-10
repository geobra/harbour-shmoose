#include "CToCxxProxy.h"

#include "Omemo.h"

extern "C" void* CToCxxProxyGetInstance() {
   return &CToCxxProxy::getInstance();
}

CToCxxProxy& CToCxxProxy::getInstance()
{
    static CToCxxProxy instance_;
    return instance_;
}

void CToCxxProxy::setOmemoPtr(Omemo* omemo)
{
    omemo_ = omemo;
}



extern "C" void CToCxxProxySendAsPepStanza(void* proxy, char* stanza)
{
   static_cast<CToCxxProxy*>(proxy)->sendAsPepStanza(stanza);
}
void CToCxxProxy::sendAsPepStanza(char* stanza)
{
    omemo_->sendAsPepStanza(stanza);
}



extern "C" void CToCxxProxySendRawMessageStanza(void* proxy, char* stanza)
{
   static_cast<CToCxxProxy*>(proxy)->sendRawMessageStanza(stanza);
}
void CToCxxProxy::sendRawMessageStanza(char* stanza)
{
    omemo_->sendRawMessageStanza(stanza);
}



void CToCxxProxySendBundleRequest(void* proxy, char* node, void* q_msg)
{
    static_cast<CToCxxProxy*>(proxy)->sendBundleRequest(node, q_msg);
}
void CToCxxProxy::sendBundleRequest(char* node, void* q_msg)
{
    omemo_->sendBundleRequest(node, q_msg);
}



void CToCxxProxyCreateAndSendBundleRequest(void* proxy, char* sender, char* bundle)
{
    static_cast<CToCxxProxy*>(proxy)->createAndSendBundleRequest(sender, bundle);
}
void CToCxxProxy::createAndSendBundleRequest(char* sender, char* bundle)
{
    omemo_->createAndSendBundleRequest(sender, bundle);
}
