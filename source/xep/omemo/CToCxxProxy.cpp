#include "CToCxxProxy.h"

#include "LurchAdapter.h"

extern "C" void* CToCxxProxyGetInstance() {
   return &CToCxxProxy::getInstance();
}

CToCxxProxy& CToCxxProxy::getInstance()
{
    static CToCxxProxy instance_;
    return instance_;
}

void CToCxxProxy::setLurchAdapterPtr(LurchAdapter* lurchAdapter)
{
    lurchAdapter_ = lurchAdapter;
}



extern "C" void CToCxxProxySendAsPepStanza(void* proxy, char* stanza)
{
   static_cast<CToCxxProxy*>(proxy)->sendAsPepStanza(stanza);
}
void CToCxxProxy::sendAsPepStanza(char* stanza)
{
    lurchAdapter_->sendAsPepStanza(stanza);
}



extern "C" void CToCxxProxySendRawMessageStanza(void* proxy, char* stanza)
{
   static_cast<CToCxxProxy*>(proxy)->sendRawMessageStanza(stanza);
}
void CToCxxProxy::sendRawMessageStanza(char* stanza)
{
    lurchAdapter_->sendRawMessageStanza(stanza);
}



void CToCxxProxySendBundleRequest(void* proxy, char* node, void* q_msg)
{
    static_cast<CToCxxProxy*>(proxy)->sendBundleRequest(node, q_msg);
}
void CToCxxProxy::sendBundleRequest(char* node, void* q_msg)
{
    lurchAdapter_->sendBundleRequest(node, q_msg);
}



void CToCxxProxyCreateAndSendBundleRequest(void* proxy, char* sender, char* bundle)
{
    static_cast<CToCxxProxy*>(proxy)->createAndSendBundleRequest(sender, bundle);
}
void CToCxxProxy::createAndSendBundleRequest(char* sender, char* bundle)
{
    lurchAdapter_->createAndSendBundleRequest(sender, bundle);
}


void CToCxxProxyShowMessageToUser(void* proxy, char* title, char* msg)
{
    static_cast<CToCxxProxy*>(proxy)->showMessageToUser(title, msg);
}
void CToCxxProxy::showMessageToUser(char* title, char* msg)
{
    //lurchAdapter_->showMessageToUser(title, msg);
}
