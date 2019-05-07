#ifndef __LURCH_H
# define __LURCH_H

# define LURCH_VERSION "0.6.7"
# define LURCH_AUTHOR "Richard Bayerle <riba@firemail.cc>"

void lurch_xml_received_cb(PurpleConnection * gc_p, xmlnode ** stanza_pp);
void lurch_account_connect_cb(PurpleAccount * acc_p);
void lurch_pep_own_devicelist_request_handler(JabberStream * js_p, const char * from, xmlnode * items_p);
void lurch_pep_bundle_for_keytransport(JabberStream * js_p, const char * from, xmlnode * items_p);
void lurch_message_encrypt_im(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp);
void lurch_message_decrypt(PurpleConnection * gc_p, xmlnode ** msg_stanza_pp);
void lurch_pep_devicelist_event_handler(JabberStream * js_p, const char * from, xmlnode * items_p);
PurpleCmdRet lurch_cmd_func(PurpleConversation * conv_p, const gchar * cmd, gchar ** args, gchar ** error, void * data_p);
char * lurch_uname_get_db_fn(const char * uname, char * which);
void lurch_bundle_request_cb(JabberStream * js_p, const char * from, JabberIqType type, const char * id, xmlnode * packet_p, gpointer data_p);
void lurch_conv_created_cb(PurpleConversation * conv_p);
int lurch_axc_get_init_ctx(char * uname, axc_context ** ctx_pp);

#endif /* __LURCH_H */
