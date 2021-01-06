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

typedef enum {
    JABBER_IQ_SET,
    JABBER_IQ_GET,
    JABBER_IQ_RESULT,
    JABBER_IQ_ERROR,
    JABBER_IQ_NONE
} JabberIqType;

static JabberStream jabberStream;

#ifdef __cplusplus
}
#endif
