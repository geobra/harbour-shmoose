#ifndef OMEMO_H
#define OMEMO_H

#include <QObject>
#include <QMap>
#include <QString>

#include <Swiften/Swiften.h>

extern "C" {
#include "axc.h"
#include "libomemo.h"
}

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = nullptr);
    ~Omemo();
    void setupWithClient(Swift::Client* client);

signals:

public slots:

private:
    void requestDeviceList(const Swift::JID& jid);
    void ownDeviceListRequestHandler(QString fromJid, QString items);

    void handleDeviceListResponse(const std::string& str);
    void publishedDeviceList(const std::string& str);
    void publishedBundle(const std::string& str);

    bool axcPrepare(QString fromJid);
    bool axcGetInitCtx(axc_context** ctx_pp);
    int bundlePublishOwn();
    int devicelistProcess(const char *uname, omemo_devicelist * dl_in_p);

    char* unameGetDbFn(const char * uname, char * which);

    void handleDataReceived(Swift::SafeByteArray data);


    Swift::Client* client_{};
    QString deviceListNodeName_{};
    QString currentNode_{};
    QString myBareJid_{};
    char* uname_{nullptr};

    int uninstall_{0};

    QMap<QString, QString> requestedDeviceListJidIdMap_{};


};

#endif // OMEMO_H
