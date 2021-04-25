#ifndef CLIENTCOMTEST_H
#define CLIENTCOMTEST_H

#include "ClientComtestCommon.h"

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class OmemoTest : public ClientComTestCommon
{
    Q_OBJECT

public:
    OmemoTest();

private slots:
    void sendMsgTest();
    void collectMsgStateLhsChanged(QString msgId, int state);
    void collectMsgStateRhsChanged(QString msgId, int state);

private:
    enum side
    {
        lhs,
        rhs
    };

    struct MsgIdState
    {
        QString msgId;
        int state;
    };

    bool destrcutiveVerfiyStateAndCountOfMsgStates(enum side theSide, const QString& msgIdFilter, QList<MsgIdState> msgsList);

    QList<MsgIdState> stateChangeMsgLhsList_;
    QList<MsgIdState> stateChangeMsgRhsList_;
};

#endif // CLIENTCOMTEST_H
