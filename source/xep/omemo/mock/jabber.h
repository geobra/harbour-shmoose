#pragma once

#include "xmlnode.h"

void jabber_pep_publish(void* foo, xmlnode *node);

// just to have this type as a mock
typedef struct JabberStream_ JabberStream;
struct JabberStream_ {
char* gc;
};
