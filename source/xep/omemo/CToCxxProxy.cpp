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
