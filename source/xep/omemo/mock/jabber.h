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

static JabberStream jabberStream;

#ifdef __cplusplus
}
#endif
