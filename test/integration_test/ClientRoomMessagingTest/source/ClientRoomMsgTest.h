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

    void sendRoomMsgTest();
    void collectMsgStateChangedLhs(QString msgId, QString jid, int state);
    void collectMsgStateChangedRhs(QString msgId, QString jid, int state);

    void collectLatestMsgRhs(QString msgId, QString jid, QString msg);

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

    struct MsgContent
    {
        QString msgId;
        QString jid;
        QString msg;
    };

    QList<MsgIdJidState> stateChangeMsgLhsList_;
    QList<MsgIdJidState> stateChangeMsgRhsList_;

    QList<MsgContent> collectedMsgRhsList_;

    bool destrcutiveVerfiyStateAndCountOfMsgStates(enum side theSide, const QString& msgIdFilter, QList<MsgIdJidState> msgsList);
};

#endif // CLIENTROOMMSGTEST_H
