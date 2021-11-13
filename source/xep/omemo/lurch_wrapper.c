#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wpadded"
#pragma GCC diagnostic ignored "-Wmissing-variable-declarations"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"

#include "lurch_wrapper.h"

#include "lurch.c"

void lurch_bundle_request_cb_wrap(JabberStream * js_p, const char * from,
                                    JabberIqType type, const char * id,
                                    xmlnode * packet_p, gpointer data_p)
{
    lurch_bundle_request_cb(js_p, from, type, id, packet_p, data_p);
}

void lurch_pep_bundle_for_keytransport_wrap(JabberStream * js_p, const char * from, xmlnode * items_p)
{
    lurch_pep_bundle_for_keytransport(js_p, from, items_p);
}

int lurch_devicelist_process_wrap(char * uname, omemo_devicelist * dl_in_p, JabberStream * js_p)
{
    return lurch_devicelist_process(uname, dl_in_p, js_p);
}

void lurch_pep_own_devicelist_request_handler_wrap(JabberStream * js_p, const char * from, xmlnode * items_p)
{
    lurch_pep_own_devicelist_request_handler(js_p, from, items_p);
}

void lurch_message_encrypt_im_wrap(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp)
{
    lurch_message_encrypt_im(gc_p, msg_stanza_pp);
}

void lurch_message_encrypt_groupchat_wrap(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp)
{
    lurch_message_encrypt_groupchat(gc_p, msg_stanza_pp);
}

void lurch_message_decrypt_wrap(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp)
{
    lurch_message_decrypt(gc_p, msg_stanza_pp);
}

PurpleCmdRet lurch_cmd_func_wrap(PurpleConversation * conv_p,
                                   const gchar * cmd,
                                   gchar ** args,
                                   gchar ** error,
                                   void * data_p)
{
    return lurch_cmd_func(conv_p, cmd, args, error, data_p);
}

void lurch_pep_devicelist_event_handler_wrap(JabberStream * js_p, const char * from, xmlnode * items_p)
{
    lurch_pep_devicelist_event_handler(js_p, from, items_p);
}

void lurch_pep_muc_user_handler_wrap(JabberStream * js_p, const char * from, const char * real_jid)
{
    lurch_pep_muc_user_handler(js_p, from, real_jid);
}

#pragma GCC diagnostic pop
