#include "jabber.h"
#include "CToCxxProxy.h"

void jabber_pep_publish(void* foo, xmlnode *node)
{
    int len = 0;
    char* str = xmlnode_to_str(node, &len);

    //forward str to omemo class (via CToCxxProxy) to be send as a pep message
    void* proxy = CToCxxProxyGetInstance();
    CToCxxProxySendAsPepStanza(proxy, str);

    g_free(str);
}

JabberIq* jabber_iq_new(JabberStream* js, JabberIqType type)
{
    // assigen xmlnode to jabberiq.node and return it
    xmlnode* node = xmlnode_new("iq");
    JabberIq* jq = malloc(sizeof (JabberIq));
    jq->node = node;
    jq->id = NULL;
    jq->q_msg = NULL;

    return jq;
}

void jabber_iq_set_id(JabberIq *jiq, char* id)
{
    jiq->id = id;
}

void jabber_iq_set_callback(JabberIq* jiq, void* cb, gpointer q_msg)
{
    jiq->q_msg = q_msg;
}

void jabber_iq_send(JabberIq* jiq_p)
{
    int len = 0;
    char* node = xmlnode_to_str(jiq_p->node, &len);
    char* id = jiq_p->id;
    void* q_msg = jiq_p->q_msg;

    //forward str to omemo class (via CToCxxProxy) to be send as a pep message
    void* proxy = CToCxxProxyGetInstance();
    CToCxxProxySendBundleRequest(proxy, node, id, q_msg);

    // FIXME free xmlnode and jiq. NOT q_msg!
}
