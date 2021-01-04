#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include "purple.h"

#include <stdio.h>
#include <stdarg.h>
#include <glib.h>
#include <string.h>

void purple_debug_info (const char *category, const char *format,...)
{
    (void) category;

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void purple_debug_error(const char *category, const char *format,...)
{
    purple_debug_info(category, format);
}

void purple_debug_warning(const char *category, const char *format,...)
{
    purple_debug_info(category, format);
}

void purple_debug_misc(const char *category, const char *format,...)
{
    purple_debug_info(category, format);
}

int purple_prefs_get_bool(char* str)
{
    return 0;
}

int purple_prefs_get_int(char* str)
{
    return 0;
}

void set_omemo_dir(const char* dir)
{
    memset(omemo_dir, '\0', OMEMO_DIR_LEN);
    size_t len = strlen(dir);
    if (len > OMEMO_DIR_LEN -1 )
    {
        len = OMEMO_DIR_LEN -1;
    }
    memcpy(omemo_dir, dir, len);
    //fprintf(stderr, "->%s\n", omemo_dir);
}

const char* purple_user_dir()
{
    return omemo_dir;
}

gchar* purple_base16_encode_chunked(u_int8_t* in, size_t size)
{
    // FIXME IMPLEMENT ME!
}

#pragma GCC diagnostic pop
