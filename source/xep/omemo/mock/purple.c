#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"

#include "purple.h"
#include "CToCxxProxy.h"

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
    memset(omemo_dir, '\0', MAX_LEN);
    size_t len = strlen(dir);
    if (len > MAX_LEN -1 )
    {
        len = MAX_LEN -1;
    }
    memcpy(omemo_dir, dir, len);
}

const char* purple_user_dir()
{
    return omemo_dir;
}

gchar* purple_base16_encode_chunked(const guchar *data, gsize len)
{
    // from libpurple: util.c
    gsize i;
    gchar *ascii = NULL;

    g_return_val_if_fail(data != NULL, NULL);
    g_return_val_if_fail(len > 0,   NULL);

    ascii = g_malloc(len * 3 + 1);

    for (i = 0; i < len; i++)
        g_snprintf(&ascii[i * 3], 4, "%02hhx:", data[i]);

    ascii[len * 3 - 1] = 0;

    return ascii;
}

void* purple_connection_get_account(void* foo)
{
    return NULL;
}

void set_fqn_name(const char* name)
{
    memset(fq_user_name, '\0', MAX_LEN);
    size_t len = strlen(name);
    if (len > MAX_LEN -1 )
    {
        len = MAX_LEN -1;
    }
    memcpy(fq_user_name, name, len);
}

void set_current_chat_partner(const char *jid)
{
    memset(current_char_partner, '\0', MAX_LEN);
    size_t len = strlen(jid);
    if (len > MAX_LEN -1 )
    {
        len = MAX_LEN -1;
    }
    memcpy(current_char_partner, jid, len);
}

char* purple_account_get_username(void* foo)
{
    return fq_user_name;
}

gboolean purple_strequal(const gchar *left, const gchar *right)
{
    return (g_strcmp0(left, right) == 0);
}

char *purple_unescape_text(const char *in)
{
    GString *ret;
    const char *c = in;

    if (in == NULL)
        return NULL;

    ret = g_string_new("");
    while (*c) {
        int len;
        const char *ent;

        if ((ent = purple_markup_unescape_entity(c, &len)) != NULL) {
            g_string_append(ret, ent);
            c += len;
        } else {
            g_string_append_c(ret, *c);
            c++;
        }
    }

    return g_string_free(ret, FALSE);
}

const char * purple_markup_unescape_entity(const char *text, int *length)
{
    const char *pln;
    int len;

    if (!text || *text != '&')
        return NULL;

#define IS_ENTITY(s)  (!g_ascii_strncasecmp(text, s, (len = sizeof(s) - 1)))

    if(IS_ENTITY("&amp;"))
        pln = "&";
    else if(IS_ENTITY("&lt;"))
        pln = "<";
    else if(IS_ENTITY("&gt;"))
        pln = ">";
    else if(IS_ENTITY("&nbsp;"))
        pln = " ";
    else if(IS_ENTITY("&copy;"))
        pln = "\302\251";      /* or use g_unichar_to_utf8(0xa9); */
    else if(IS_ENTITY("&quot;"))
        pln = "\"";
    else if(IS_ENTITY("&reg;"))
        pln = "\302\256";      /* or use g_unichar_to_utf8(0xae); */
    else if(IS_ENTITY("&apos;"))
        pln = "\'";
    else if(text[1] == '#' && (g_ascii_isxdigit(text[2]) || text[2] == 'x')) {
        static char buf[7];
        const char *start = text + 2;
        char *end;
        guint64 pound;
        int base = 10;
        int buflen;

        if (*start == 'x') {
            base = 16;
            start++;
        }

        pound = g_ascii_strtoull(start, &end, base);
        if (pound == 0 || pound > INT_MAX || *end != ';') {
            return NULL;
        }

        len = (end - text) + 1;

        buflen = g_unichar_to_utf8((gunichar)pound, buf);
        buf[buflen] = '\0';
        pln = buf;
    }
    else
        return NULL;

    if (length)
        *length = len;
    return pln;
}

JabberStream* purple_connection_get_protocol_data(void* foo)
{
    return &jabberStream;
}

int purple_plugins_find_with_id(char* foo)
{
    return 0;
}

void purple_signal_emit(int foo, char* what, char* bar, xmlnode** node)
{
    if (strcmp(what, "jabber-sending-xmlnode") == 0)
    {
        int len = 0;
        char* str = xmlnode_to_str(*node, &len);

#ifndef UNIT_TEST
        void* proxy = CToCxxProxyGetInstance();
        CToCxxProxySendRawMessageStanza(proxy, str);
#endif
    }
}

void purple_conv_present_error(char* from, void* foo, char* msg)
{
    // FIXME forward to user interface
    fprintf(stderr, "msg from %s: %s\n", from, msg);
}

void* purple_conversation_get_account(PurpleConversation * conv_p)
{
    return NULL;
}

void* purple_conversation_get_gc(PurpleConversation * conv_p)
{
    return NULL;
}

enum PurpleConvTyp purple_conversation_get_type (PurpleConversation * conv_p)
{
    // FIXME for now, always return
    return PURPLE_CONV_TYPE_IM;
}

char* purple_conversation_get_name(PurpleConversation * conv_p)
{
    return current_char_partner;
}

void purple_conversation_autoset_title(PurpleConversation * conv_p)
{

}

void purple_conversation_write(PurpleConversation * conv_p, char* id, char* msg, enum PurpleFlags fags, time_t time)
{
    // FIXME Forward as replay to user
    fprintf(stderr, "%s: %s\n", id, msg);
}

void lurch_topic_update_im(PurpleConversation * conv_p)
{}

void lurch_topic_update_chat(PurpleConversation * conv_p)
{}

#pragma GCC diagnostic pop
