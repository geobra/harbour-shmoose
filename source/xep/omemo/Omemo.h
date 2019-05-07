#ifndef OMEMO_H
#define OMEMO_H

#include "purple.h"

#include <Swiften/Swiften.h>

#include <QObject>
#include <glib.h>

// ugly access from pure C into C++ object
extern "C"
{
void* OmemoGetInstance();
void OmemoPepRequestItem(void* omemo, const char *uname, const char *node);
const char *OmemoGetPath(void* omemo);
const char *OmemoGetUname(void* omemo);
void OmemoPublishRawXml(void* omemo, std::string xml);
void OmemoBundleRequest(void* omemo, const std::string& node);
void OmemoStoreLurchQueuedMsgPtr(void* omemo, void* ptr);
char *getDataPathName();
}

class Omemo : public QObject
{
    Q_OBJECT
public:
    static Omemo& getInstance();
    ~Omemo();

    void setupWithClient(Swift::Client *client);
    void pepRequestItem(const char *uname, const char *node);
    void publishPepRawXml(const std::string& rawXml);
    void bundleRequest(const std::string& node);
    void storeLurchQueuedMsgPtr(void* ptr);

    bool isOmemoAvailableForBarJid(const std::string& jid);

    QString encryptMessage(const QString& msg);
    QString decryptMessage(const QString& msg);

    static QString stripParentNodeAtElement(const QString& childElement, const QString &xml);
    static QString getChildFromNode(const QString& childElement, const QString &xml);

    const char *getPath();
    const char* getUname();

public slots:
    void currentChatPartner(QString jid);

private:
    explicit Omemo(QObject *parent = 0);

    bool removeFile(char* file);

    void handleDataReceived(Swift::SafeByteArray data);
    void handleConnected();

    void publisedPepHandler(const std::string& str);
    void requestBundleHandler(const std::string& str);

    void requestDeviceList(const Swift::JID& jid);
    void handleDeviceListResponse(const std::string& str);

    QString getFirstNodeNameOfXml(const QString& xml);

    void cleanupDeviceList();

    //void requestBundle(unsigned int device_id, const Swift::JID& jid);
    void handlePepResponse(const std::string& str);

    QString getValueForElementInNode(const QString& node, const QString& xmlNode, const QString& elementString);

    struct iqCallBackData
    {
        JabberIq* iq;
        JabberIqCallback* cb;
        gpointer data;
    };

    QList<iqCallBackData> iqCallBackData_;

    Swift::Client* client_;

    PurpleAccount purpleAccount_;
    PurpleConnection purpleConnection_;
    PurpleConversation purpleConversation_;
    JabberStream jabberStream_;

    std::string jidOfRequestedDeviceList_;
    QStringList checkedDeviceList_;

    // temp data for the request bundle id callback
    QString requestBundleFrom_;
    QString requestBundleStanzaId_;
    void* lurchQueuedMsgPtr_;

    char *username_;
    static const unsigned int partnerNameLength = 100;
    char partnername_[partnerNameLength];
    char* omemoDir_;

private slots:
    void shotAfterDelay();
    void uninstall();

signals:
    void omemoAvailableFor(QString jid);

};

extern "C" {
//void purple_debug_error (const char *category, const char *format,...);
//void purple_debug_info(const char *category, const char *format,...);
//void purple_debug_warning (const char *category, const char *format,...);
//void purple_debug_misc (const char *category, const char *format,...);

// used to add features that will be advertised in dico#info
void jabber_add_feature(const gchar *ns, JabberFeatureEnabled cb);

// get bare jid
char *jabber_get_bare_jid(const char *jid);

// create new jabber iq stanza
JabberIq *jabber_iq_new(JabberStream *js, JabberIqType type);

// send jabber iq stanza
void jabber_iq_send(JabberIq *iq);

// set a callback to a request
void jabber_iq_set_callback(JabberIq *iq, JabberIqCallback *cb, gpointer data);

// set id in the iq stanza
void jabber_iq_set_id(JabberIq *iq, const char *id);

// publish the xml pep node
void jabber_pep_publish(JabberStream *js, xmlnode *publish);

// register handler for the device list (in pep)
void jabber_pep_register_handler(const char *xmlns, JabberPEPHandler handlerfunc);

// request a pep item to a pep handler cb
void jabber_pep_request_item(JabberStream *js, const char *to, const char *node, const char *id, JabberPEPHandler cb);

// request an account setting
gboolean purple_account_get_bool(const PurpleAccount *account, const char *name, gboolean default_value);

// returns the accounts connection
PurpleConnection *purple_account_get_connection(const PurpleAccount *account);

// returns the accounts protocol id
const char *purple_account_get_protocol_id(const PurpleAccount *account);

// returns the accounts user name
const char *purple_account_get_username(const PurpleAccount *account);

// returns whether or not the account is connected
gboolean purple_account_is_connected(const PurpleAccount *account);

// Sets a protocol-specific boolean setting for an account.
void purple_account_set_bool(PurpleAccount *account, const char *name, gboolean value);

// returns a list of all enabled accounts
GList *purple_accounts_get_all_active(void);

// returns the accounts subsystem handle
void *purple_accounts_get_handle(void);

// Converts a chunk of binary data to a chunked base-16 representation
gchar *purple_base16_encode_chunked(const guchar *data, gsize len);

// register to libpurple
PurpleCmdId purple_cmd_register(const gchar *cmd, const gchar *args, PurpleCmdPriority p, PurpleCmdFlag f,
                             const gchar *prpl_id, PurpleCmdFunc func, const gchar *helpstr, void *data);

// returns the connections account
PurpleAccount *purple_connection_get_account(const PurpleConnection *gc);

// gets the protocoll data from a connection
void *purple_connection_get_protocol_data(const PurpleConnection *connection);

// writes to a chat
void purple_conv_chat_write(PurpleConvChat *chat, const char *who,
                          const char *message, PurpleMessageFlags flags,
                          time_t mtime);

// sets the chat title
void purple_conversation_autoset_title(PurpleConversation *conv);

// returns the purple account for the conversation
PurpleAccount *purple_conversation_get_account(const PurpleConversation *conv);

// returns the chat data for the conversation
PurpleConvChat *purple_conversation_get_chat_data(const PurpleConversation *conv);

// returns to purple connection for the conversation
PurpleConnection *purple_conversation_get_gc(const PurpleConversation *conv);

// returns the conversation name for the purple conversation
const char *purple_conversation_get_name(const PurpleConversation *conv);

// returns the conversations title
const char *purple_conversation_get_title(const PurpleConversation *conv);

// returns the specified conversations type
PurpleConversationType purple_conversation_get_type(const PurpleConversation *conv);

// creates a new conversation for the specified type
PurpleConversation *purple_conversation_new(PurpleConversationType type,
                                        PurpleAccount *account,
                                        const char *name);

// sets the title to a conversation
void purple_conversation_set_title(PurpleConversation *conv, const char *title);

// get the conversations handle
void *purple_conversations_get_handle(void);

// write a (plain) text message to a conversation
void purple_conversation_write(PurpleConversation *conv, const char *who,
        const char *message, PurpleMessageFlags flags,
        time_t mtime);

// present an error to the user
gboolean purple_conv_present_error(const char *who, PurpleAccount *account, const char *what);

// finds conversation by type, name and account
PurpleConversation *purple_find_conversation_with_account(
        PurpleConversationType type, const char *name,
        const PurpleAccount *account);

// finds a plugin with the specified id
PurplePlugin *purple_plugins_find_with_id(const char *id);

// connects a signal handler for an object
gulong purple_signal_connect_priority(void *instance, const char *signal,
    void *handle, PurpleCallback func, void *data, int priority);

// emits a signal
void purple_signal_emit(void *instance, const char *signal, ...);

// returns the settings dir
const char *purple_user_dir(void);

}

#endif
