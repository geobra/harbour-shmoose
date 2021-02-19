#include "lurch_wrapper.h"

#include <glib.h>

void lurch_bundle_request_cb_wrap(JabberStream * js_p, const char * from,
                                    JabberIqType type, const char * id,
                                    xmlnode * packet_p, gpointer data_p)
{
    //lurch_bundle_request_cb(js_p, from, type, id, packet_p, data_p);
}

void lurch_pep_bundle_for_keytransport_wrap(JabberStream * js_p, const char * from, xmlnode * items_p)
{
    //lurch_pep_bundle_for_keytransport(js_p, from, items_p);
}

int lurch_devicelist_process_wrap(char * uname, omemo_devicelist * dl_in_p, JabberStream * js_p)
{
    //return lurch_devicelist_process(uname, dl_in_p, js_p);
}

void lurch_pep_own_devicelist_request_handler_wrap(JabberStream * js_p, const char * from, xmlnode * items_p)
{
    //lurch_pep_own_devicelist_request_handler(js_p, from, items_p);
}

void lurch_message_encrypt_im_wrap(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp)
{
    //lurch_message_encrypt_im(gc_p, msg_stanza_pp);
}

void lurch_message_decrypt_wrap(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp)
{
    //lurch_message_decrypt(gc_p, msg_stanza_pp);
}

PurpleCmdRet lurch_cmd_func_wrap(PurpleConversation * conv_p,
                                   const gchar * cmd,
                                   gchar ** args,
                                   gchar ** error,
                                   void * data_p)
{
    //return lurch_cmd_func(conv_p, cmd, args, error, data_p);
}

void lurch_pep_devicelist_event_handler_wrap(JabberStream * js_p, const char * from, xmlnode * items_p)
{
    //lurch_pep_devicelist_event_handler(js_p, from, items_p);
}

