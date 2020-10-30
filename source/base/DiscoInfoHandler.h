#ifndef DISCOINFOHANDLER_H
#define DISCOINFOHANDLER_H

#include <QObject>
#include <Swiften/Swiften.h>

class HttpFileUploadManager;
class MamManager;

class DiscoInfoHandler : public QObject
{
    Q_OBJECT
public:
    explicit DiscoInfoHandler(HttpFileUploadManager* httpFileUploadManager, MamManager* mamManager, QObject *parent = 0);
    ~DiscoInfoHandler();

    void setupWithClient(Swift::Client* client);

signals:
    void serverHasHttpUpload_(bool);
    void serverHasMam_(bool);

public slots:

private:
    void handleServerDiscoInfoResponse(std::shared_ptr<Swift::DiscoInfo> info, Swift::ErrorPayload::ref error);
    void handleDiscoServiceWalker(const Swift::JID & jid, std::shared_ptr<Swift::DiscoInfo> info);
    void cleanupDiscoServiceWalker();
    void handleServerDiscoItemsResponse(std::shared_ptr<Swift::DiscoItems> items, Swift::ErrorPayload::ref error);

    HttpFileUploadManager* httpFileUploadManager_;
    MamManager* mamManager_;
    Swift::Client* client_;

    QList<std::shared_ptr<Swift::DiscoServiceWalker> > danceFloor_;
};

#endif // DISCOINFOHANDLER_H
