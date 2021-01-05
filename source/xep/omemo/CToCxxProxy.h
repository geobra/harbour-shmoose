#ifndef CTOCXXPROXY_H
#define CTOCXXPROXY_H

// ugly access from pure C into C++ object
extern "C"
{
void* CToCxxProxyGetInstance();
void CToCxxProxySendAsPepStanza(void* proxy, char* stanza);
}

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

#endif // CTOCXXPROXY_H
