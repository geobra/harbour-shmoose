#pragma once

extern "C" {
#include "libomemo.h"
}

struct lurch_queued_msg {
  omemo_message * om_msg_p;
  GList * recipient_addr_l_p;
  GList * no_sess_l_p;
  GHashTable * sess_handled_p;
};

typedef struct lurch_addr {
  char * jid;
  uint32_t device_id;
};
