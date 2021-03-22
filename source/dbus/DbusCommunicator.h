#ifndef DBUSCOMMUNICATOR_H
#define DBUSCOMMUNICATOR_H

#include "Shmoose.h"
#include "RosterController.h"

#include <QObject>

class DbusCommunicator : public QObject
{
    Q_OBJECT

public:
    explicit DbusCommunicator(const QString& path, const QString& name, QObject * const parent = nullptr);
    void setXmpClient(Shmoose* shmoose);
    void setupConnections();

public slots:
    Q_SCRIPTABLE bool tryToConnect(const QString& jid, const QString& pass);
    Q_SCRIPTABLE bool requestRoster();
    Q_SCRIPTABLE bool addContact(const QString& jid, const QString& name);
    Q_SCRIPTABLE bool joinRoom(const QString& jid, const QString& name);
    Q_SCRIPTABLE bool removeRoom(const QString& jid);
    Q_SCRIPTABLE bool removeContact(const QString& jid);
    Q_SCRIPTABLE bool sendMsg(const QString& jid, const QString& msg);
    Q_SCRIPTABLE bool sendFile(const QString& jid, const QString& path);
    Q_SCRIPTABLE bool setCurrentChatPartner(QString jid);
    Q_SCRIPTABLE bool disconnectFromServer();
    Q_SCRIPTABLE bool reConnect();
    Q_SCRIPTABLE bool requestRosterList();
    Q_SCRIPTABLE bool addForcePlainMsgForJid(const QString& jid);
    Q_SCRIPTABLE bool rmForcePlainMsgForJid(const QString& jid);

    Q_SCRIPTABLE bool quitClient();

signals:
    void signalConnectionStateChanged();
    void signalNewRosterEntry();
    void signalRoomJoined(QString, QString);
    void signalLatestMsg(QString, QString, QString);
    void signalMsgState(QString, int);
    void signalRoomMsgState(QString, QString, int);
    void signalDownloadFinished(QString);
    void signalMsgSent(QString);
    void signalRosterEntry(QString jid, QString name, int subscription, int availability, QString status, QString image, bool isGroup);
    void signalRosterListDone();
    void signalMucRoomRemoved(QString);
    void signalSubscriptionUpdated(int);
    void signalReceivedDeviceListOfJid(QString);

private slots:
    void slotConnectionStateChanged();
    void slotGotRosterEntry();
    bool slotNewRoomJoin(QString jid, QString name);
    void slotForwaredReceivedMsgToDbus(QString id, QString jid, QString message);
    void slotForwardMsgStateToDbus(QString msgId, int msgState);
    void slotForwardRoomMsgStateToDbus(QString msgId, QString jid, int msgState);
    void slotForwardDownloadMsgToDbus(QString);
    void slotForwardMsgSentToDbus(QString);
    void slotForwardMucRoomRemoved(QString);
    void slotForwardSubscriptionUpdate(RosterItem::Subscription sub);
    void slotForwardReceivedDeviceListOfJid(QString jid);

private:
    Shmoose* shmoose_;

    QString dbusObjectPath_;
    QString dbusServiceName_;
};

#endif // DBUSCOMMUNICATOR_H
