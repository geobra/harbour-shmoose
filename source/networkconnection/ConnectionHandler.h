#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <QObject>
#include <Swiften/Swiften.h>

class ReConnectionHandler;
class IpHeartBeatWatcher;
class XmppPingController;

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHandler(QObject *parent = 0);
    ~ConnectionHandler();

    void setupWithClient(Swift::Client* client);
    void setHasInetConnection(bool connected);

    bool isConnected();

signals:
    void connectionStateChanged();
    void signalHasInetConnection(bool connected);

    void signalInitialConnectionEstablished();

public slots:
    void tryStablishServerConnection();
    void slotAppGetsActive(bool active);

private slots:
    void tryReconnect();

private:
    void handleConnected();
    void handleDisconnected(const boost::optional<Swift::ClientError> &error);

    bool connected_;
    bool initialConnectionSuccessfull_;
    bool hasInetConnection_;
    bool appIsActive_;

    Swift::Client* client_;
    ReConnectionHandler *reConnectionHandler_;
    IpHeartBeatWatcher *ipHeartBeatWatcher_;
    XmppPingController *xmppPingController_;
};

#endif // CONNECTIONHANDLER_H
