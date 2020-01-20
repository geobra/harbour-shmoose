#ifndef DBUSCOMMUNICATOR_H
#define DBUSCOMMUNICATOR_H

#include "Shmoose.h"
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
    Q_SCRIPTABLE bool sendMsg(const QString& jid, const QString& msg);
    Q_SCRIPTABLE bool sendFile(const QString& jid, const QString& path);
    Q_SCRIPTABLE bool setCurrentChatPartner(QString jid);
    Q_SCRIPTABLE bool disconnectFromServer();
    Q_SCRIPTABLE bool reConnect();

    Q_SCRIPTABLE bool quitClient();

signals:
    void signalConnected();
    void signalNewRosterEntry();
    void signalLatestMsg(QString, QString, QString);
    void signalMsgState(QString, int);
    void signalDownloadFinished(QString);

private slots:
    void slotClientConnected();
    void slotGotRosterEntry();
    void slotForwaredReceivedMsgToDbus(QString id, QString jid, QString message);
    void slotForwardMsgStateToDbus(QString msgId, int msgState);
    void slotForwardDownloadMsgToDbus(QString);

private:
    Shmoose* shmoose_;

    QString dbusObjectPath_;
    QString dbusServiceName_;
};

#endif // DBUSCOMMUNICATOR_H
