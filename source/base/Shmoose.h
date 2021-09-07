#ifndef SHMOOSE_H
#define SHMOOSE_H

#include "Persistence.h"
#include "Settings.h"

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
class DiscoInfoHandler;
class MamManager;
class StanzaId;
class LurchAdapter;

class Shmoose : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
    Q_PROPERTY(Persistence* persistence READ getPersistence NOTIFY persistenceChanged)
    Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(Settings* settings READ getSettings NOTIFY settingsChanged)

public:
    Shmoose(Swift::NetworkFactories* networkFactories, QObject *parent = 0);
    ~Shmoose();

    Q_INVOKABLE void mainDisconnect();
    Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
    Q_INVOKABLE void reConnect();
    Q_INVOKABLE void setCurrentChatPartner(QString const &jid);
    Q_INVOKABLE QString getCurrentChatPartner();

    Q_INVOKABLE QString getAttachmentPath();
    Q_INVOKABLE void setHasInetConnection(bool connected_);
    Q_INVOKABLE void setAppIsActive(bool active);

    Q_INVOKABLE void joinRoom(QString const &roomJid, QString const &roomName);
    Q_INVOKABLE void removeRoom(QString const &roomJid);

    Q_INVOKABLE QString getLocalFileForUrl(const QString& str);

    Q_INVOKABLE bool canSendFile();
    Q_INVOKABLE QString getVersion();

    Q_INVOKABLE bool isOmemoUser(const QString& jid);

    Q_INVOKABLE void saveAttachment(const QString &msg);

    bool connectionState() const;

public slots:
    void sendMessage(QString toJid, QString message, QString type, QString msgId);
    void sendMessage(QString message, QString type);
    void sendFile(QString toJid, QString file);
    void sendFile(QUrl file);
    void attachmentUploadFailed(QString msgId);

private slots:
    void sendReadNotification(bool active);
    void intialSetupOnFirstConnection();

    void slotAboutToQuit();

signals:
    void rosterControllerChanged();
    void persistenceChanged();
    void settingsChanged();

    void connectionStateChanged();

    void signalCanSendFile(bool);

    void signalShowMessage(QString headline, QString body);
    void signalShowStatus(QString headline, QString body);

    void signalHasInetConnection(bool connected);
    void signalAppGetsActive(bool active);

private:
#ifdef DBUS
public:
#endif

    void requestHttpUploadSlot();
    void handleHttpUploadResponse(const std::string response);

    RosterController* getRosterController();
    Persistence* getPersistence();
    Settings* getSettings();

    Swift::Client* client_{nullptr};
    Swift::ClientXMLTracer* tracer_;
    Swift::SoftwareVersionResponder* softwareVersionResponder_;
    Swift::NetworkFactories *netFactories_;

    RosterController* rosterController_;
    Persistence* persistence_;
    Settings* settings_;

    StanzaId *stanzaId_;
    ConnectionHandler* connectionHandler_;
    LurchAdapter* lurchAdapter_;
    MessageHandler* messageHandler_;
    HttpFileUploadManager* httpFileUploadManager_;
    MamManager *mamManager_;
    MucManager *mucManager_;
    DiscoInfoHandler* discoInfoHandler_;

    QString jid_;
    QString password_;

    const QString version_;
};

#endif
