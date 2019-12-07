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

public slots:
    Q_SCRIPTABLE bool tryToConnect(const QString& jid, const QString& pass);
    Q_SCRIPTABLE bool requestRoster();
    Q_SCRIPTABLE bool addContact(const QString& jid, const QString& name);
    Q_SCRIPTABLE bool sendMsg(const QString& jid, const QString& msg);
    Q_SCRIPTABLE bool requestLatestMsgForJid(const QString& jid);

signals:
    void connected();
    void newRosterEntry();
    void signalMsgChanged();
    void signalLatestMsg(QString, int);

private slots:
    void clientConnected();
    void gotRosterEntry();
    void msgChanged();

private:
    Shmoose* shmoose_;

    QString dbusObjectPath_;
    QString dbusServiceName_;
};

#endif // DBUSCOMMUNICATOR_H
