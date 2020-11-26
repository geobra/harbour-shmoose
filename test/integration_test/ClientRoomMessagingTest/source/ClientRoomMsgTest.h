#ifndef CLIENTROOMMSGTEST_H
#define CLIENTROOMMSGTEST_H

#include "ClientComtestCommon.h"

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class ClientRoomMsgTest : public ClientComTestCommon
{
    Q_OBJECT

public:
    ClientRoomMsgTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void sendRoomMsgTest();
    void collectMsgStateChangedLhs(QString msgId, QString jid, int state);
    void collectMsgStateChangedRhs(QString msgId, QString jid, int state);

private:
    DbusInterfaceWrapper* interfaceMhs_;
    const QString user3jid_;

    enum side
    {
        lhs,
        mhs,
        rhs
    };

    struct MsgIdJidState
    {
        QString msgId;
        QString jid;
        int state;
    };

    QList<MsgIdJidState> stateChangeMsgLhsList_;
    QList<MsgIdJidState> stateChangeMsgRhsList_;

    bool destrcutiveVerfiyStateAndCountOfMsgStates(enum side theSide, const QString& msgIdFilter, QList<MsgIdJidState> msgsList);
};

#endif // CLIENTROOMMSGTEST_H
