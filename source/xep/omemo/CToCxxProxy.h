#ifndef CTOCXXPROXY_H
#define CTOCXXPROXY_H

// ugly access from pure C into C++ object
#ifdef __cplusplus
extern "C"
{
#endif
void* CToCxxProxyGetInstance();
void CToCxxProxySendAsPepStanza(void* proxy, char* stanza);
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

private:
    CToCxxProxy() = default;
    Omemo* omemo_;
};
#endif

#endif // CTOCXXPROXY_H
