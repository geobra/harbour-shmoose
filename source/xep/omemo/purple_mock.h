#ifndef PURPLE_MOCK_H
#define PURPLE_MOCK_H

gboolean purple_account_get_bool(const PurpleAccount *account, const char *name, gboolean default_value)
{
    (void) account;

    gboolean return_value = default_value;

    if (strncmp("lurch_initialised", name, 18) == 0)
    {
        void* omemo = OmemoGetInstance();
        const char* uname = OmemoGetUname(omemo);

        char which[] = LURCH_DB_NAME_AXC;
        char* pWhich = which;

        char* axcDb = lurch_uname_get_db_fn(uname, pWhich);

        if (axcDb != NULL)
        {
            if( access( axcDb, F_OK ) != -1 )
            {
                return_value = true;
            }
            else
            {
                return_value = false;
            }

            free(axcDb);
        }
    }

    return return_value;
}

PurpleConnection *purple_account_get_connection(const PurpleAccount *account)
{
    return account->gc;
}

const char *purple_account_get_protocol_id(const PurpleAccount *account)
{
    (void) account;

    return JABBER_PROTOCOL_ID;
}

const char *purple_account_get_username(const PurpleAccount *account)
{
    return account->username;
}

gboolean purple_account_is_connected(const PurpleAccount *account)
{
    (void) account;

    return true;
}

void purple_account_set_bool(PurpleAccount *account, const char *name, gboolean value)
{
    // ignore
    // used to set the lurch initialized flag.
    // shmoose does not use a flag, but checks for the axolot dir instead
    (void) account;
    (void) name;
    (void) value;
}

GList *purple_accounts_get_all_active(void)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    // FIXME return null

    exit(255);

    return NULL;
}

void *purple_accounts_get_handle(void)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;

    exit(255);

    // FIXME
    return NULL;
}

gchar *purple_base16_encode_chunked(const guchar *data, gsize len)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) data;
    (void) len;

    exit(255);

    return NULL;
    // FIXME
}

PurpleCmdId purple_cmd_register(const gchar *cmd, const gchar *args, PurpleCmdPriority p, PurpleCmdFlag f,
                             const gchar *prpl_id, PurpleCmdFunc func, const gchar *helpstr, void *data)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) cmd;
    (void) args;
    (void) p;
    (void) f;
    (void) prpl_id;
    (void) func;
    (void) helpstr;
    (void) data;

    exit(255);

    return 0;
    // FIXME make own register method
}

PurpleAccount *purple_connection_get_account(const PurpleConnection *gc)
{
    return gc->account;
}

void *purple_connection_get_protocol_data(const PurpleConnection *connection)
{
    return connection->proto_data;
}

void purple_conv_chat_write(PurpleConvChat *chat, const char *who,
                          const char *message, PurpleMessageFlags flags,
                          time_t mtime)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) chat;
    (void) who;
    (void) message;
    (void) flags;
    (void) mtime;

    exit(255);

    // FIXME
}

void purple_conversation_autoset_title(PurpleConversation *conv)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) conv;

    exit(255);

    // IGNORE
}

PurpleAccount *purple_conversation_get_account(const PurpleConversation *conv)
{
    return conv->account;
}

PurpleConvChat *purple_conversation_get_chat_data(const PurpleConversation *conv)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) conv;

    exit(255);

    return NULL;
    // FIXME
}

PurpleConnection *purple_conversation_get_gc(const PurpleConversation *conv)
{
    return conv->account->gc;
}

const char *purple_conversation_get_name(const PurpleConversation *conv)
{
    return conv->name;
}

const char *purple_conversation_get_title(const PurpleConversation *conv)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) conv;

    exit(255);

    return NULL;
    // FIXME IGNORE
}

PurpleConversationType purple_conversation_get_type(const PurpleConversation *conv)
{
    return conv->type;
}

PurpleConversation *purple_conversation_new(PurpleConversationType type,
                                        PurpleAccount *account,
                                        const char *name)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void)  type;
    (void) account;
    (void) name;

    exit(255);

    return NULL;
    // FIXME
}

void purple_conversation_set_title(PurpleConversation *conv, const char *title)
{
    (void) conv;

    // FIXME update title of msg page!
    fprintf(stderr, "new title: %s\n", title);
}

void *purple_conversations_get_handle(void)
{
    //std::cout << "implement me! " << __FUNCTION__ << std::endl;
    return NULL;
    // IGNORE
}

void purple_conversation_write(PurpleConversation *conv, const char *who,
        const char *message, PurpleMessageFlags flags,
        time_t mtime)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) conv;
    (void) who;
    (void) message;
    (void) flags;
    (void) mtime;

    //exit(255);

    // FIXME this is used at a lot of places in lurch.c
}

gboolean purple_conv_present_error(const char *who, PurpleAccount *account, const char *what)
{
    // used to show real messages to the user
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) who;
    (void) account;
    (void) what;

    // FIXME present error to user!
    std::cout << __FUNCTION__ << ": " << who << ", " << what << std::endl;

    return true;
}

PurpleConversation *purple_find_conversation_with_account(
        PurpleConversationType type, const char *name,
        const PurpleAccount *account)
{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) type;
    (void) name;
    (void) account;

    exit(255);

    return NULL;
    // FIXME
}

PurplePlugin *purple_plugins_find_with_id(const char *id)
{
    // not used in shmoose integration
    (void) id;
    return NULL;
}

gulong purple_signal_connect_priority(void *instance, const char *signal,
    void *handle, PurpleCallback func, void *data, int priority)

{
    std::cout << "implement me! " << __FUNCTION__ << std::endl;
    (void) instance;
    (void) signal;
    (void) handle;
    (void) func;
    (void) data;
    (void) priority;

    exit(255);

    return 0;
    // FIXME
}

void purple_signal_emit(void *instance, const char *signal, ...)
{
    // not used in shmoose integration
    (void) instance;
    (void) signal;
}

const char *purple_user_dir(void)
{
    //return System::getOmemoPath().toStdString().c_str();
    void* omemo = OmemoGetInstance();
    return OmemoGetPath(omemo);
}

#endif // PURPLE_MOCK_H
