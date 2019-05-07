#ifndef JABBER_MOCK_H
#define JABBER_MOCK_H

void jabber_add_feature(const gchar *ns, JabberFeatureEnabled cb)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) ns;
    (void) cb;
    // used for disco#info pep...
    // IGNORE
}

char *jabber_get_bare_jid(const char *jid)
{
    char* return_value;

    std::string s(jid);
    size_t pos = s.find("/");
    if (pos != std::string::npos)
    {
        return_value = strndup(jid, pos);
    }
    else
    {
        return_value = strdup(jid);
    }

    return return_value;
}

char *jabber_get_next_id(JabberStream *js)
{
    return g_strdup_printf("purple%x", js->next_id++);
}

JabberIq *jabber_iq_new(JabberStream *js, JabberIqType type)
{
    JabberIq *iq;

    iq = g_new0(JabberIq, 1);

    iq->type = type;

    iq->node = xmlnode_new("iq");
    switch(iq->type)
    {
        case JABBER_IQ_SET:
            xmlnode_set_attrib(iq->node, "type", "set");
            break;
        case JABBER_IQ_GET:
            xmlnode_set_attrib(iq->node, "type", "get");
            break;
        case JABBER_IQ_ERROR:
            xmlnode_set_attrib(iq->node, "type", "error");
            break;
        case JABBER_IQ_RESULT:
            xmlnode_set_attrib(iq->node, "type", "result");
            break;
        case JABBER_IQ_NONE:
            /* this shouldn't ever happen */
            break;
    }

    iq->js = js;

    if(type == JABBER_IQ_GET || type == JABBER_IQ_SET)
    {
        iq->id = jabber_get_next_id(js);
        xmlnode_set_attrib(iq->node, "id", iq->id);
    }

    return iq;
}

void jabber_iq_free(JabberIq *iq)
{
    g_return_if_fail(iq != NULL);

    g_free(iq->id);
    xmlnode_free(iq->node);
    g_free(iq);
}

void jabber_iq_send(JabberIq *iq)
{
    // FIXME not tested!!!
    // Only used from lurch_bundle_request_do in lurch 0.6.7
    xmlnode* xNode = iq->node;

    int len = 0;
    std::string sNode = xmlnode_to_str(xNode, &len);
    //std::cout << sNode << std::endl;

    void* omemo = OmemoGetInstance();
    OmemoBundleRequest(omemo, sNode);

    jabber_iq_free(iq);
}

void jabber_iq_set_callback(JabberIq *iq, JabberIqCallback *cb, gpointer data)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) cb;
    (void) data;

    const char* id = xmlnode_get_attrib(iq->node, "id");
    // FIXME make a list of id - cb  - data
    // check for iq response. parse id and call cb (callback) with data (gpointer)
    // iqCallBackData_
    // adjust OmemoBundleRequest to use this data!

    // FIXME pass id and data pointer into omemo.cpp
    // then catch incoming stanza ids and search for a match.
    // call lurch_bundle_request_cb from omemo.cpp with the derefered data pointer

    // as an alternative, request bundle for each contact beforehand...

    void* omemo = OmemoGetInstance();
    OmemoStoreLurchQueuedMsgPtr(omemo, data);

    //exit(255);
}

void jabber_iq_set_id(JabberIq *iq, const char *id)
{
    g_free(iq->id);

    if(id)
    {
        xmlnode_set_attrib(iq->node, "id", id);
        iq->id = g_strdup(id);
    } else
    {
        xmlnode_remove_attrib(iq->node, "id");
        iq->id = NULL;
    }
}

void jabber_pep_publish(JabberStream *js, xmlnode *publish)
{
    (void) js;
    std::cout << __FUNCTION__ << std::endl;

    int len;
    char* xml = xmlnode_to_str(publish, &len);
    std::string node(xml);
    free(xml);

    void* omemo = OmemoGetInstance();
    OmemoPublishRawXml(omemo, node);
}

void jabber_pep_register_handler(const char *xmlns, JabberPEPHandler handlerfunc)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) xmlns;
    (void) handlerfunc;

    // FIXME
    exit(255);
}

void jabber_pep_request_item(JabberStream *js, const char *to, const char *node, const char *id, JabberPEPHandler cb)
{
    (void) js;

    //qDebug() << "to:" << QString().fromLocal8Bit(to) << ", node:" << QString().fromLocal8Bit(node) << ", id: " << QString().fromLocal8Bit(id);

    // TODO implement PEP request and install swift callback, which calls the supplied cb

    //https://stackoverflow.com/questions/14815274/how-to-call-a-c-method-from-c

    // cb in lurch v 0.6.7 can be lurch_pep_own_devicelist_request_handler or lurch_pep_bundle_for_keytransport
    // swift does not support binding a request to a specific cb
    // Omemo::pepRequestItem installs the response handler Omemo::handlePepResponse.
    // There has to be a parsing for the answer type and the cb's have to be called accordingly.
    // THIS IS A MANUAL TASK :-(.

    // Here we check for the used version of lurch. If it is not the expected one, we print an error to the dev
    // and exit to make clear that the cb's may have to be updated.

    // FIXME check LURCH_VERSION

    void* omemo = OmemoGetInstance();
    OmemoPepRequestItem(omemo, to, node);
}


#endif // JABBER_MOCK_H
