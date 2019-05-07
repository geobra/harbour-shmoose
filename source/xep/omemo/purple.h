#ifndef PURPLE_H
#define PURPLE_H

/*
 * All the header stuff and functions, lurch needs as an interface to libpurple.
 * All structs are reduced as small as possible to get lurch compiled and functional
 * with a minimum footprint.
 */

#define PURPLE_CALLBACK(x) (x)
#define purple_signal_connect(_A, _B, _C, _D, _E) ((void) (_A, _B, _C, _D, _E))

// ------------------------------------------------------------------------
#include <glib.h>

#include "xmlnode.h"

typedef int PurpleCmdId;
typedef struct _PurpleConnection PurpleConnection;

typedef struct _PurpleAccount      PurpleAccount;
struct _PurpleAccount
{
    char *username;             /**< The username.                          */
#if 0
    char *alias;                /**< How you appear to yourself.            */
    char *password;             /**< The account password.                  */
    char *user_info;            /**< User information.                      */

    char *buddy_icon_path;      /**< The buddy icon's non-cached path.      */

    gboolean remember_pass;     /**< Remember the password.                 */

    char *protocol_id;          /**< The ID of the protocol.                */
#endif
    PurpleConnection *gc;         /**< The connection handle.                 */
#if 0
    gboolean disconnecting;     /**< The account is currently disconnecting */

    GHashTable *settings;       /**< Protocol-specific settings.            */
    GHashTable *ui_settings;    /**< UI-specific settings.                  */

    PurpleProxyInfo *proxy_info;  /**< Proxy information.  This will be set   */
                                /*   to NULL when the account inherits      */
                                /*   proxy settings from global prefs.      */

    /*
     * TODO: Supplementing the next two linked lists with hash tables
     * should help performance a lot when these lists are long.  This
     * matters quite a bit for protocols, where all your
     * buddies are added to your permit list.  Currently we have to
     * iterate through the entire list if we want to check if someone
     * is permitted or denied.  We should do this for 3.0.0.
     * Or maybe use a GTree.
     */
    GSList *permit;             /**< Permit list.                           */
    GSList *deny;               /**< Deny list.                             */
    PurplePrivacyType perm_deny;  /**< The permit/deny setting.               */

    GList *status_types;        /**< Status types.                          */

    PurplePresence *presence;     /**< Presence.                              */
    PurpleLog *system_log;        /**< The system log                         */

    void *ui_data;              /**< The UI can put data here.              */
    PurpleAccountRegistrationCb registration_cb;
    void *registration_cb_user_data;

    gpointer priv;              /**< Pointer to opaque private data. */
#endif
};

struct _PurpleConnection
{
#if 0
    PurplePlugin *prpl;
    PurpleConnectionFlags flags;
    PurpleConnectionState state;
#endif
    PurpleAccount *account;
#if 0
    char *password;
    int inpa;
    GSList *buddy_chats;
#endif
    void *proto_data;
#if 0
    char *display_name;
    guint keepalive;
    gboolean wants_to_die;

    guint disconnect_timeout;
    time_t last_received;
#endif
};

gboolean purple_strequal(const gchar *left, const gchar *right);
char *purple_unescape_text(const char *in);
const char * purple_markup_unescape_entity(const char *text, int *length);

void purple_debug_info (const char *category, const char *format,...);
void purple_debug_error(const char *category, const char *format,...);
void purple_debug_warning(const char *category, const char *format,...);
void purple_debug_misc(const char *category, const char *format,...);

// ####################### jabber.h ##########################
typedef struct _JabberStream JabberStream;
struct _JabberStream
{
#if 0
    int fd;

    PurpleSrvTxtQueryData *srv_query_data;

    xmlParserCtxt *context;
    xmlnode *current;

    struct {
        guint8 major;
        guint8 minor;
    } protocol_version;

    JabberSaslMech *auth_mech;
    gpointer auth_mech_data;

    /**
     * The header from the opening <stream/> tag.  This being NULL is treated
     * as a special condition in the parsing code (signifying the next
     * stanza started is an opening stream tag), and its being missing on
     * the stream header is treated as a fatal error.
     */
    char *stream_id;
    JabberStreamState state;

    GHashTable *buddies;

    /*
     * This boolean was added to eliminate a heinous bug where we would
     * get into a loop with the server and move a buddy back and forth
     * from one group to another.
     *
     * The sequence goes something like this:
     * 1. Our resource and another resource both approve an authorization
     *    request at the exact same time.  We put the buddy in group A and
     *    the other resource put the buddy in group B.
     * 2. The server receives the roster add for group B and sends us a
     *    roster push.
     * 3. We receive this roster push and modify our local blist.  This
     *    triggers us to send a roster add for group B.
     * 4. The server recieves our earlier roster add for group A and sends
     *    us a roster push.
     * 5. We receive this roster push and modify our local blist.  This
     *    triggers us to send a roster add for group A.
     * 6. The server receives our earlier roster add for group B and sends
     *    us a roster push.
     * (repeat steps 3 through 6 ad infinitum)
     *
     * This boolean is used to short-circuit the sending of a roster add
     * when we receive a roster push.
     *
     * See these bug reports:
     * http://trac.adiumx.com/ticket/8834
     * http://developer.pidgin.im/ticket/5484
     * http://developer.pidgin.im/ticket/6188
     */
    gboolean currently_parsing_roster_push;

    GHashTable *chats;
    GList *chat_servers;
    PurpleRoomlist *roomlist;
    GList *user_directories;

    GHashTable *iq_callbacks;
#endif
    int next_id;
#if 0

    GList *bs_proxies;
    GList *oob_file_transfers;
    GList *file_transfers;

    time_t idle;
    time_t old_idle;

    /** When we last pinged the server, so we don't ping more
     *  often than once every minute.
     */
    time_t last_ping;

    JabberID *user;
    JabberBuddy *user_jb;
#endif

    PurpleConnection *gc;
#if 0
    PurpleSslConnection *gsc;

    gboolean registration;

    char *initial_avatar_hash;
    char *avatar_hash;
    GSList *pending_avatar_requests;

    GSList *pending_buddy_info_requests;

    PurpleCircBuffer *write_buffer;
    guint writeh;

    gboolean reinit;

    JabberCapabilities server_caps;
    gboolean googletalk;
    char *server_name;

    char *gmail_last_time;
    char *gmail_last_tid;

    char *serverFQDN;

#ifdef HAVE_CYRUS_SASL
    sasl_conn_t *sasl;
    sasl_callback_t *sasl_cb;
    sasl_secret_t *sasl_secret;
    const char *current_mech;
    int auth_fail_count;

    int sasl_state;
    int sasl_maxbuf;
    GString *sasl_mechs;
#endif

    gboolean unregistration;
    PurpleAccountUnregistrationCb unregistration_cb;
    void *unregistration_user_data;

    gboolean vcard_fetched;
    /* Timer at login to push updated avatar */
    guint vcard_timer;

    /* Entity Capabilities hash */
    char *caps_hash;

#endif
    /* does the local server support PEP? */
    gboolean pep;

#if 0
    /* Is Buzz enabled? */
    gboolean allowBuzz;

    /* A list of JabberAdHocCommands supported by the server */
    GList *commands;

    /* last presence update to check for differences */
    JabberBuddyState old_state;
    char *old_msg;
    int old_priority;
    char *old_avatarhash;

    /* same for user tune */
    char *old_artist;
    char *old_title;
    char *old_source;
    char *old_uri;
    int old_length;
    char *old_track;

    char *certificate_CN;

    /* A purple timeout tag for the keepalive */
    guint keepalive_timeout;
    guint max_inactivity;
    guint inactivity_timer;

    PurpleSrvResponse *srv_rec;
    guint srv_rec_idx;
    guint max_srv_rec_idx;

    /* BOSH stuff */
    PurpleBOSHConnection *bosh;

    /**
     * This linked list contains PurpleUtilFetchUrlData structs
     * for when we lookup buddy icons from a url
     */
    GSList *url_datas;

    /* keep a hash table of JingleSessions */
    GHashTable *sessions;

    /* maybe this should only be present when USE_VV? */
    gchar *stun_ip;
    int stun_port;
    PurpleDnsQueryData *stun_query;

    /* stuff for Google's relay handling */
    gchar *google_relay_token;
    gchar *google_relay_host;
    GList *google_relay_requests; /* the HTTP requests to get */
                                                /* relay info */

    /* facebook quirks */
    gboolean facebook_roster_cleanup_performed;
#endif
};

typedef gboolean (JabberFeatureEnabled)(JabberStream *js, const gchar *ns);
typedef struct _JabberFeature
{
    gchar *ns;
    JabberFeatureEnabled *is_enabled;
} JabberFeature;


// ######################## iq.h ##########################
//#include "iq.h"
typedef enum {
    JABBER_IQ_SET,
    JABBER_IQ_GET,
    JABBER_IQ_RESULT,
    JABBER_IQ_ERROR,
    JABBER_IQ_NONE
} JabberIqType;


typedef void (JabberIqCallback)(JabberStream *js, const char *from,
                                JabberIqType type, const char *id,
                                xmlnode *packet, gpointer data);

typedef struct _JabberIq JabberIq;
struct _JabberIq {
    JabberIqType type;
    char *id;
    xmlnode *node;

    JabberIqCallback *callback;
    gpointer callback_data;

    JabberStream *js;
};

// ######################## conversation.h ####################

typedef struct _PurpleConversation      PurpleConversation;
typedef struct _PurpleConvChat          PurpleConvChat;
typedef struct _PurpleConvIm            PurpleConvIm;

typedef enum
{
    PURPLE_CONV_TYPE_UNKNOWN = 0, /**< Unknown conversation type. */
    PURPLE_CONV_TYPE_IM,          /**< Instant Message.           */
    PURPLE_CONV_TYPE_CHAT,        /**< Chat room.                 */
    PURPLE_CONV_TYPE_MISC,        /**< A misc. conversation.      */
    PURPLE_CONV_TYPE_ANY          /**< Any type of conversation.  */

} PurpleConversationType;

typedef enum
{
    PURPLE_MESSAGE_SEND        = 0x0001, /**< Outgoing message.        */
    PURPLE_MESSAGE_RECV        = 0x0002, /**< Incoming message.        */
    PURPLE_MESSAGE_SYSTEM      = 0x0004, /**< System message.          */
    PURPLE_MESSAGE_AUTO_RESP   = 0x0008, /**< Auto response.           */
    PURPLE_MESSAGE_ACTIVE_ONLY = 0x0010,  /**< Hint to the UI that this
                                            message should not be
                                            shown in conversations
                                            which are only open for
                                            internal UI purposes
                                            (e.g. for contact-aware
                                             conversations).           */
    PURPLE_MESSAGE_NICK        = 0x0020, /**< Contains your nick.      */
    PURPLE_MESSAGE_NO_LOG      = 0x0040, /**< Do not log.              */
    PURPLE_MESSAGE_WHISPER     = 0x0080, /**< Whispered message.       */
    PURPLE_MESSAGE_ERROR       = 0x0200, /**< Error message.           */
    PURPLE_MESSAGE_DELAYED     = 0x0400, /**< Delayed message.         */
    PURPLE_MESSAGE_RAW         = 0x0800, /**< "Raw" message - don't
                                            apply formatting         */
    PURPLE_MESSAGE_IMAGES      = 0x1000, /**< Message contains images  */
    PURPLE_MESSAGE_NOTIFY      = 0x2000, /**< Message is a notification */
    PURPLE_MESSAGE_NO_LINKIFY  = 0x4000, /**< Message should not be auto-
                                           linkified @since 2.1.0 */
    PURPLE_MESSAGE_INVISIBLE   = 0x8000, /**< Message should not be displayed */
    PURPLE_MESSAGE_REMOTE_SEND = 0x10000 /**< Message sent from another location,
                                           not an echo of a local one
                                           @since 2.12.0 */
} PurpleMessageFlags;


typedef enum
{
    PURPLE_CONV_UPDATE_ADD = 0, /**< The buddy associated with the conversation
                                   was added.   */
    PURPLE_CONV_UPDATE_REMOVE,  /**< The buddy associated with the conversation
                                   was removed. */
    PURPLE_CONV_UPDATE_ACCOUNT, /**< The purple_account was changed. */
    PURPLE_CONV_UPDATE_TYPING,  /**< The typing state was updated. */
    PURPLE_CONV_UPDATE_UNSEEN,  /**< The unseen state was updated. */
    PURPLE_CONV_UPDATE_LOGGING, /**< Logging for this conversation was
                                   enabled or disabled. */
    PURPLE_CONV_UPDATE_TOPIC,   /**< The topic for a chat was updated. */
    /*
     * XXX These need to go when we implement a more generic core/UI event
     * system.
     */
    PURPLE_CONV_ACCOUNT_ONLINE,  /**< One of the user's accounts went online.  */
    PURPLE_CONV_ACCOUNT_OFFLINE, /**< One of the user's accounts went offline. */
    PURPLE_CONV_UPDATE_AWAY,     /**< The other user went away.                */
    PURPLE_CONV_UPDATE_ICON,     /**< The other user's buddy icon changed.     */
    PURPLE_CONV_UPDATE_TITLE,
    PURPLE_CONV_UPDATE_CHATLEFT,

    PURPLE_CONV_UPDATE_FEATURES  /**< The features for a chat have changed */

} PurpleConvUpdateType;

struct _PurpleConversation
{
    PurpleConversationType type;  /**< The type of conversation.          */
    PurpleAccount *account;       /**< The user using this conversation.  */


    char *name;                 /**< The name of the conversation.      */
    char *title;                /**< The window title.                  */
#if 0
    gboolean logging;           /**< The status of logging.             */

    GList *logs;                /**< This conversation's logs           */
#endif
    union
    {
        PurpleConvIm   *im;       /**< IM-specific data.                  */
        PurpleConvChat *chat;     /**< Chat-specific data.                */
        void *misc;             /**< Misc. data.                        */

    } u;
#if 0
    PurpleConversationUiOps *ui_ops;           /**< UI-specific operations. */
    void *ui_data;                           /**< UI-specific data.       */

    GHashTable *data;                        /**< Plugin-specific data.   */

    PurpleConnectionFlags features; /**< The supported features */
    GList *message_history;         /**< Message history, as a GList of PurpleConvMessage's */
#endif
};

struct _PurpleConvChat
{
    PurpleConversation *conv;          /**< The parent conversation.      */

    GList *in_room;                  /**< The users in the room.
                                      *   @deprecated Will be removed in 3.0.0
                                      */
    GList *ignored;                  /**< Ignored users.                */
    char  *who;                      /**< The person who set the topic. */
    char  *topic;                    /**< The topic.                    */
    int    id;                       /**< The chat ID.                  */
    char *nick;                      /**< Your nick in this chat.       */

    gboolean left;                   /**< We left the chat and kept the window open */
    GHashTable *users;               /**< Hash table of the users in the room.
                                      *   @since 2.9.0
                                      */
};

struct _PurpleConvIm
{
    PurpleConversation *conv;            /**< The parent conversation.     */

    //PurpleTypingState typing_state;      /**< The current typing state.    */
    guint  typing_timeout;             /**< The typing timer handle.     */
    time_t type_again;                 /**< The type again time.         */
    guint  send_typed_timeout;         /**< The type again timer handle. */

    //PurpleBuddyIcon *icon;               /**< The buddy icon.              */
};

// ##################### request.h ##################
typedef enum
{
    PURPLE_REQUEST_INPUT = 0,
    PURPLE_REQUEST_CHOICE,
    PURPLE_REQUEST_ACTION,
    PURPLE_REQUEST_FIELDS,
    PURPLE_REQUEST_FILE,
    PURPLE_REQUEST_FOLDER
} PurpleRequestType;


// ##################### chat.h #####################
typedef struct _JabberChat {
    JabberStream *js;
    char *room;
    char *server;
    char *handle;
    GHashTable *components;
    int id;
    PurpleConversation *conv;
    gboolean muc;
    gboolean xhtml;
    PurpleRequestType config_dialog_type;
    void *config_dialog_handle;
    GHashTable *members;
    gboolean left;
    time_t joined;
} JabberChat;

typedef struct _JabberChatMember {
    char *handle;
    char *jid;
} JabberChatMember;

JabberChat *jabber_chat_find_by_conv(PurpleConversation *conv);


// ##################### cmds.h #########################
typedef enum _PurpleCmdRet {
    PURPLE_CMD_RET_OK,       /**< Everything's okay; Don't look for another command to call. */
    PURPLE_CMD_RET_FAILED,   /**< The command failed, but stop looking.*/
    PURPLE_CMD_RET_CONTINUE /**< Continue, looking for other commands with the same name to call. */
} PurpleCmdRet;

typedef enum _PurpleCmdPriority {
    PURPLE_CMD_P_VERY_LOW  = -1000,
    PURPLE_CMD_P_LOW       =     0,
    PURPLE_CMD_P_DEFAULT   =  1000,
    PURPLE_CMD_P_PRPL      =  2000,
    PURPLE_CMD_P_PLUGIN    =  3000,
    PURPLE_CMD_P_ALIAS     =  4000,
    PURPLE_CMD_P_HIGH      =  5000,
    PURPLE_CMD_P_VERY_HIGH =  6000
} PurpleCmdPriority;

typedef enum _PurpleCmdFlag {
    /** Command is usable in IMs. */
    PURPLE_CMD_FLAG_IM               = 0x01,
    /** Command is usable in multi-user chats. */
    PURPLE_CMD_FLAG_CHAT             = 0x02,
    /** Command is usable only for a particular prpl. */
    PURPLE_CMD_FLAG_PRPL_ONLY        = 0x04,
    /** Incorrect arguments to this command should be accepted anyway. */
    PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS = 0x08
} PurpleCmdFlag;

typedef PurpleCmdRet (*PurpleCmdFunc)(PurpleConversation *, const gchar *cmd,
                                  gchar **args, gchar **error, void *data);



// #################### plugin.h ########################
typedef struct _PurplePlugin           PurplePlugin;
typedef struct _PurplePluginInfo       PurplePluginInfo;

struct _PurplePlugin
{
    gboolean native_plugin;                /**< Native C plugin.          */
    gboolean loaded;                       /**< The loaded state.         */
    void *handle;                          /**< The module handle.        */
    char *path;                            /**< The path to the plugin.   */
    PurplePluginInfo *info;                  /**< The plugin information.   */
    char *error;
    void *ipc_data;                        /**< IPC data.                 */
    void *extra;                           /**< Plugin-specific data.     */
    gboolean unloadable;                   /**< Unloadable                */
    GList *dependent_plugins;              /**< Plugins depending on this */

    void (*_purple_reserved1)(void);
    void (*_purple_reserved2)(void);
    void (*_purple_reserved3)(void);
    void (*_purple_reserved4)(void);
};

struct _PurplePluginInfo
{
    unsigned int magic;
    unsigned int major_version;
    unsigned int minor_version;
    //PurplePluginType type;
    char *ui_requirement;
    unsigned long flags;
    GList *dependencies;
    //PurplePluginPriority priority;

    char *id;
    char *name;
    char *version;
    char *summary;
    char *description;
    char *author;
    char *homepage;

    /**
     * If a plugin defines a 'load' function, and it returns FALSE,
     * then the plugin will not be loaded.
     */
    gboolean (*load)(PurplePlugin *plugin);
    gboolean (*unload)(PurplePlugin *plugin);
    void (*destroy)(PurplePlugin *plugin);

    void *ui_info; /**< Used only by UI-specific plugins to build a preference screen with a custom UI */
    void *extra_info;
    //PurplePluginUiInfo *prefs_info; /**< Used by any plugin to display preferences.  If #ui_info has been specified, this will be ignored. */

    /**
     * This callback has a different use depending on whether this
     * plugin type is PURPLE_PLUGIN_STANDARD or PURPLE_PLUGIN_PROTOCOL.
     *
     * If PURPLE_PLUGIN_STANDARD then the list of actions will show up
     * in the Tools menu, under a submenu with the name of the plugin.
     * context will be NULL.
     *
     * If PURPLE_PLUGIN_PROTOCOL then the list of actions will show up
     * in the Accounts menu, under a submenu with the name of the
     * account.  context will be set to the PurpleConnection for that
     * account.  This callback will only be called for online accounts.
     */
    GList *(*actions)(PurplePlugin *plugin, gpointer context);

    void (*_purple_reserved1)(void);
    void (*_purple_reserved2)(void);
    void (*_purple_reserved3)(void);
    void (*_purple_reserved4)(void);
};

typedef enum
{
    PURPLE_PLUGIN_UNKNOWN  = -1,  /**< Unknown type.    */
    PURPLE_PLUGIN_STANDARD = 0,   /**< Standard plugin. */
    PURPLE_PLUGIN_LOADER,         /**< Loader plugin.   */
    PURPLE_PLUGIN_PROTOCOL        /**< Protocol plugin. */

} PurplePluginType;

#define PURPLE_PRIORITY_DEFAULT     0
#define PURPLE_PRIORITY_HIGHEST  9999
#define PURPLE_PLUGIN_MAGIC 5 /* once we hit 6.0.0 I think we can remove this */

#define PURPLE_INIT_PLUGIN(pluginname, initfunc, plugininfo)

// ########################## pep.h ########################
gboolean jabber_pep_namespace_only_when_pep_enabled_cb(JabberStream *js, const gchar *ns);
typedef void (JabberPEPHandler)(JabberStream *js, const char *from, xmlnode *items);

// ######################### version.h ###################
/** The major version of the running libpurple. */
#define PURPLE_MAJOR_VERSION (2)
/** The minor version of the running libpurple. */
#define PURPLE_MINOR_VERSION (12)
/** The micro version of the running libpurple. */
#define PURPLE_MICRO_VERSION (0)

// ######################### signals.h ###################
typedef void (*PurpleCallback)(void);


// ###################### conversation.h #################
JabberChat *jabber_chat_find_by_conv(PurpleConversation *conv);

#endif // PURPLE_H
