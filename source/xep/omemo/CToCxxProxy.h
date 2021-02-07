#ifndef CTOCXXPROXY_H
#define CTOCXXPROXY_H

// ugly access from pure C into C++ object
#ifdef __cplusplus
extern "C"
{
#endif
void* CToCxxProxyGetInstance();
void CToCxxProxySendAsPepStanza(void* proxy, char* stanza);
void CToCxxProxySendRawMessageStanza(void* proxy, char* stanza);
void CToCxxProxySendBundleRequest(void* proxy, char* node, void *q_msg);
void CToCxxProxyCreateAndSendBundleRequest(void* proxy, char* sender, char* bundle);
void CToCxxProxyShowMessageToUser(void* proxy, char *title, char* msg);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class Omemo;

class CToCxxProxy
{
public:
    static CToCxxProxy& getInstance();
    void setOmemoPtr(Omemo* omemo);

    void sendAsPepStanza(char* stanza);
    void sendRawMessageStanza(char* stanza);
    void sendBundleRequest(char* node, void* q_msg);
    void createAndSendBundleRequest(char* sender, char* bundle);
    void showMessageToUser(char* title, char* msg);

private:
    CToCxxProxy() = default;
    Omemo* omemo_;
};
#endif

#endif // CTOCXXPROXY_H
