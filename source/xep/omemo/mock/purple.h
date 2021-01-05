#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "jabber.h"

#include <stdio.h>
#include <stdarg.h>
#include <glib.h>
#include <string.h>

#define MAX_LEN 256
static char omemo_dir[MAX_LEN];
static char fq_user_name[MAX_LEN];

void purple_debug_info (const char *category, const char *format,...);

void purple_debug_error(const char *category, const char *format,...);

void purple_debug_warning(const char *category, const char *format,...);

void purple_debug_misc(const char *category, const char *format,...);

int purple_prefs_get_bool(char* str);

int purple_prefs_get_int(char* str);

void set_omemo_dir(const char* dir);

const char* purple_user_dir();

gchar* purple_base16_encode_chunked(u_int8_t* in, size_t size);

void* purple_connection_get_account(void* foo);

void set_fqn_name(const char *name);
char* purple_account_get_username(void *foo);

gboolean purple_strequal(const gchar *left, const gchar *right);

char *purple_unescape_text(const char *in);

const char * purple_markup_unescape_entity(const char *text, int *length);

#pragma GCC diagnostic pop
