#ifndef SHMOOSE_H
#define SHMOOSE_H

#include "Persistence.h"

#include <QObject>
#include <QStringList>
#include <QList>

#include <Swiften/Swiften.h>

class ConnectionHandler;
class MessageHandler;
class RosterController;
class Persistence;
class HttpFileUploadManager;
class MucManager;

class Shmoose : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
	Q_PROPERTY(Persistence* persistence READ getPersistence NOTIFY persistenceChanged)
    Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateChanged)

public:
	Shmoose(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Shmoose();

	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
	Q_INVOKABLE void setCurrentChatPartner(QString const &jid);
    Q_INVOKABLE QString getCurrentChatPartner();

	Q_INVOKABLE bool checkSaveCredentials();
	Q_INVOKABLE void saveCredentials(bool save);
	Q_INVOKABLE QString getJid();
	Q_INVOKABLE QString getPassword();

	Q_INVOKABLE QString getAttachmentPath();
    Q_INVOKABLE void setHasInetConnection(bool connected_);
    Q_INVOKABLE void setAppIsActive(bool active);

    Q_INVOKABLE void joinRoom(QString const &roomJid, QString const &roomName);
    Q_INVOKABLE void removeRoom(QString const &roomJid);

    Q_INVOKABLE QString getVersion();

	bool connectionState() const;

public slots:
	void sendMessage(QString const &toJid, QString const &message, const QString &type);
	void sendFile(QString const &toJid, QString const &file);

private slots:
    void sendReadNotification(bool active);
    void intialSetupOnFirstConnection();

    void slotAboutToQuit();

signals:
	void rosterControllerChanged();
	void persistenceChanged();

    void connectionStateChanged();

    void signalShowMessage(QString headline, QString body);

    void signalHasInetConnection(bool connected);
    void signalAppGetsActive(bool active);

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
    void handlePresenceChanged(Swift::Presence::ref presence);

    void handleServerDiscoInfoResponse(boost::shared_ptr<Swift::DiscoInfo> info, Swift::ErrorPayload::ref error);
    void handleDiscoServiceWalker(const Swift::JID & jid, boost::shared_ptr<Swift::DiscoInfo> info);
	void cleanupDiscoServiceWalker();
    void handleServerDiscoItemsResponse(boost::shared_ptr<Swift::DiscoItems> items, Swift::ErrorPayload::ref error);

	void requestHttpUploadSlot();
	void handleHttpUploadResponse(const std::string response);

    RosterController* getRosterController();
    Persistence* getPersistence();

	Swift::Client* client_;
	Swift::ClientXMLTracer* tracer_;
	Swift::SoftwareVersionResponder* softwareVersionResponder_;
	Swift::NetworkFactories *netFactories_;

	RosterController* rosterController_;
	Persistence* persistence_;

	Swift::GetDiscoItemsRequest::ref discoItemReq_;
	QList<boost::shared_ptr<Swift::DiscoServiceWalker> > danceFloor_;

    ConnectionHandler* connectionHandler_;
    MessageHandler* messageHandler_;
	HttpFileUploadManager* httpFileUploadManager_;
    MucManager *mucManager_;

	QString jid_;
	QString password_;

    const QString version_;
};

#endif
