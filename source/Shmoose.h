#ifndef KAIDAN_H
#define KAIDAN_H

#include <QObject>
#include <QStringList>

#include <Swiften/Swiften.h>

#include "EchoPayloadParserFactory.h"
#include "EchoPayloadSerializer.h"
#include "Persistence.h"


class RosterController;
class Persistence;
class HttpFileUploadManager;
class DownloadManager;

class Shmoose : public QObject
{
	Q_OBJECT

	Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
	Q_PROPERTY(Persistence* persistence READ getPersistence NOTIFY persistenceChanged)
	Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateConnected NOTIFY connectionStateDisconnected)

public:
	Shmoose(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
	~Shmoose();

	Q_INVOKABLE void mainDisconnect();
	Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
	Q_INVOKABLE void setCurrentChatPartner(QString const &jid);

	Q_INVOKABLE bool checkSaveCredentials();
	Q_INVOKABLE void saveCredentials(bool save);
	Q_INVOKABLE QString getJid();
	Q_INVOKABLE QString getPassword();

	Q_INVOKABLE QString getAttachmentPath();

	bool connectionState() const;

public slots:
	void sendMessage(QString const &toJid, QString const &message, const QString &type);
	void sendFile(QString const &toJid, QString const &file);


signals:
	void rosterControllerChanged();
	void persistenceChanged();

	void connectionStateConnected();
	void connectionStateDisconnected();

private:
	void handlePresenceReceived(Swift::Presence::ref presence);
	void handleConnected();
    void handleDisconnected(const boost::optional<ClientError> &error);
	void handleMessageReceived(Swift::Message::ref message);
	void handleServerDiscoInfoResponse(boost::shared_ptr<DiscoInfo> info, ErrorPayload::ref error);

	void requestHttpUploadSlot();
	void handleHttpUploadResponse(const std::string response);

	bool connected;

	RosterController* getRosterController();
	Persistence* getPersistence();

	Swift::Client* client_;
	Swift::ClientXMLTracer* tracer_;
	Swift::SoftwareVersionResponder* softwareVersionResponder_;
	EchoPayloadParserFactory echoPayloadParserFactory_;
	EchoPayloadSerializer echoPayloadSerializer_;
	Swift::NetworkFactories *netFactories_;

	RosterController* rosterController_;
	Persistence* persistence_;

	HttpFileUploadManager* httpFileUploadManager_;
	DownloadManager *downloadManager_;

	QString jid_;
	QString password_;
};

#endif
