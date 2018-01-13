#ifndef OMEMO_H
#define OMEMO_H

extern "C" {
#include "libomemo.h"
#include "libomemo_crypto.h"
#include "libomemo_storage.h"

#include "axc.h"
#include "axc_store.h"
}

#include <QObject>
#include <Swiften/Swiften.h>

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = 0);
    ~Omemo();

    void setupWithClient(Swift::Client *client);

signals:

public slots:

private:
    void requestDeviceList(const Swift::JID &jid);
    void handleMessageReceived(Swift::Message::ref message);
    void handleDeviceListResponse(const std::string &str);

    int lurch_axc_prepare(char * uname);
    int lurch_axc_get_init_ctx(char * uname, axc_context ** ctx_pp);
    char* lurch_uname_get_db_fn(const char * uname, char * which);
    void lurch_pep_own_devicelist_request_handler(const std::string &items_p);
    int lurch_bundle_publish_own();
    int lurch_devicelist_process(char * uname, omemo_devicelist * dl_in_p /*, JabberStream * js_p*/);

    void purple_debug_error (const char *category, const char *format,...);
    void purple_debug_info(const char *category, const char *format,...);

    Swift::Client* client_;
    Swift::JID actualBareJid;

    int uninstall;
};

#endif // OMEMO_H
