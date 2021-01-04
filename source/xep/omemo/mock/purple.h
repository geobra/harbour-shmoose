#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include <stdio.h>
#include <stdarg.h>
#include <glib.h>
#include <string.h>

#define OMEMO_DIR_LEN 256
static char omemo_dir[OMEMO_DIR_LEN];

void purple_debug_info (const char *category, const char *format,...);

void purple_debug_error(const char *category, const char *format,...);

void purple_debug_warning(const char *category, const char *format,...);

void purple_debug_misc(const char *category, const char *format,...);

int purple_prefs_get_bool(char* str);

int purple_prefs_get_int(char* str);

void set_omemo_dir(const char* dir);

const char* purple_user_dir();

gchar* purple_base16_encode_chunked(u_int8_t* in, size_t size);

#pragma GCC diagnostic pop
