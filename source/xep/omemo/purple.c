#include <stdio.h>
#include<stdarg.h>

#include "purple.h"

void purple_debug_info (const char *category, const char *format,...)
{
    va_list args;

    va_start(args, format);

#ifdef SFOS
    // FIXME check if this open close loop is a performance issue.
    char* dataPath = getDataPathName();
    FILE* fp = fopen(dataPath, "wa");
    if(fp != NULL)
    {
        vfprintf(fp, format, args);
        fclose(fp);
    }
    free(dataPath);
#else
    vfprintf(stderr, format, args);
#endif

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


// ##################### pep.c #######################

gboolean jabber_pep_namespace_only_when_pep_enabled_cb(JabberStream *js, const gchar *namespace)
{
    return js->pep;
}

JabberChat *jabber_chat_find_by_conv(PurpleConversation *conv)
{
#if 0
    PurpleAccount *account = purple_conversation_get_account(PURPLE_CONVERSATION(conv));
    PurpleConnection *gc = purple_account_get_connection(account);
    JabberStream *js;
    int id;
    if (!gc)
        return NULL;
    js = purple_connection_get_protocol_data(gc);
    id = purple_chat_conversation_get_id(conv);
    return jabber_chat_find_by_id(js, id);
#endif
    // FIXME implement me for group chats!!!
    return NULL;
}
