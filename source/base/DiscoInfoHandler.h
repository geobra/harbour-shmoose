#ifndef DISCOINFOHANDLER_H
#define DISCOINFOHANDLER_H

#include <QObject>
#include <Swiften/Swiften.h>

class HttpFileUploadManager;

class DiscoInfoHandler : public QObject
{
    Q_OBJECT
public:
    explicit DiscoInfoHandler(HttpFileUploadManager* httpFileUploadManager, QObject *parent = 0);
    ~DiscoInfoHandler();

    void setupWithClient(Swift::Client* client);

signals:

public slots:

private:
    void handleServerDiscoInfoResponse(boost::shared_ptr<Swift::DiscoInfo> info, Swift::ErrorPayload::ref error);
    void handleDiscoServiceWalker(const Swift::JID & jid, boost::shared_ptr<Swift::DiscoInfo> info);
    void cleanupDiscoServiceWalker();
    void handleServerDiscoItemsResponse(boost::shared_ptr<Swift::DiscoItems> items, Swift::ErrorPayload::ref error);

    HttpFileUploadManager* httpFileUploadManager_;
    Swift::Client* client_;

    Swift::GetDiscoItemsRequest::ref discoItemReq_;
    QList<boost::shared_ptr<Swift::DiscoServiceWalker> > danceFloor_;
};

#endif // DISCOINFOHANDLER_H
