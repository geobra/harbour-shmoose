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

private:
    DbusInterfaceWrapper* interfaceMhs_;
    const QString user3jid_;
};

#endif // CLIENTROOMMSGTEST_H
