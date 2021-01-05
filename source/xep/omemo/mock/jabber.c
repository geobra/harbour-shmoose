#include "jabber.h"

void jabber_pep_publish(void* foo, xmlnode *node)
{
    int len = 0;
    char* str = xmlnode_to_str(node, &len);

    //forward str to omemo class (via CToCxxProxy) to be send as a pep message
    void* proxy = CToCxxProxyGetInstance();
    CToCxxProxySendAsPepStanza(proxy, str);

    g_free(str);
}
