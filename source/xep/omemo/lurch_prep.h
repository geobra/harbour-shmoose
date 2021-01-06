#pragma once

#include "libomemo.h"
#include "LurchTypes.h"

#include "jabber.h"
#include "axc.h"


/* FIXME
 * - reduce to a minimum when porting is done
 * - make static as much as possible
 * */

int lurch_queued_msg_create(omemo_message * om_msg_p,
                                   GList * recipient_addr_l_p,
                                   GList * no_sess_l_p,
                                   lurch_queued_msg ** qmsg_pp);


int lurch_axc_prepare(char * uname);

char * lurch_queue_make_key_string_s(const char * name, const char * device_id);

int lurch_key_encrypt(const lurch_addr * recipient_addr_p,
                             const uint8_t * key_p,
                             size_t key_len,
                             axc_context * axc_ctx_p,
                             axc_buf ** key_ct_buf_pp);

int lurch_axc_prepare(char * uname);

void lurch_addr_list_destroy_func(gpointer data);

int lurch_msg_encrypt_for_addrs(omemo_message * om_msg_p, GList * addr_l_p, axc_context * axc_ctx_p);

int lurch_queued_msg_is_handled(const lurch_queued_msg * qmsg_p);

void lurch_queued_msg_destroy(lurch_queued_msg * qmsg_p);

int lurch_bundle_publish_own(JabberStream * js_p);

