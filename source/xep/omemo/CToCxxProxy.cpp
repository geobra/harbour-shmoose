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

void CToCxxProxySendBundleRequest(void* proxy, char* node, char* id, void* q_msg)
{
    static_cast<CToCxxProxy*>(proxy)->sendBundleRequest(node, id, q_msg);
}

void CToCxxProxy::sendBundleRequest(char* node, char* id, void* q_msg)
{
    omemo_->sendBundleRequest(node, id, q_msg);
}
