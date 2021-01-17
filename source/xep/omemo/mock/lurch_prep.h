#pragma once

#include "libomemo.h"
#include "jabber.h"
#include "purple.h"

typedef void PurpleConnection;

void lurch_bundle_request_cb(JabberStream * js_p, const char * from,
                                    JabberIqType type, const char * id,
                                    xmlnode * packet_p, gpointer data_p);

void lurch_pep_bundle_for_keytransport(JabberStream * js_p, const char * from, xmlnode * items_p);

int lurch_devicelist_process(char * uname, omemo_devicelist * dl_in_p, JabberStream * js_p);

void lurch_pep_own_devicelist_request_handler(JabberStream * js_p, const char * from, xmlnode * items_p);

void lurch_message_encrypt_im(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp);

void lurch_message_decrypt(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp);

enum PurpleCmdRet lurch_cmd_func(PurpleConversation * conv_p,
                                   const gchar * cmd,
                                   gchar ** args,
                                   gchar ** error,
                                   void * data_p);
