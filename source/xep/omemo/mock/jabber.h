#pragma once

#include "xmlnode.h"

#ifdef __cplusplus
extern "C"
{
#endif

void jabber_pep_publish(void* foo, xmlnode *node);

// just to have this type as a mock
typedef struct JabberStream_ JabberStream;
struct JabberStream_ {
char* gc;
};

typedef struct JabberIq_ JabberIq;
struct JabberIq_ {
    xmlnode* node;
    //char* id;
    void* q_msg;
};

typedef enum {
    JABBER_IQ_SET,
    JABBER_IQ_GET,
    JABBER_IQ_RESULT,
    JABBER_IQ_ERROR,
    JABBER_IQ_NONE
} JabberIqType;

static JabberStream jabberStream;

JabberIq* jabber_iq_new(JabberStream *js, JabberIqType type);
void jabber_iq_set_id(JabberIq *jiq, char* id);
void jabber_iq_set_callback(JabberIq* jiq, void* cb, gpointer q_msg);
void jabber_iq_send(JabberIq* jiq_p);
void jabber_pep_request_item(JabberStream* js, char* sender, char* bundle, void* foo, void* cb);


#ifdef __cplusplus
}
#endif
