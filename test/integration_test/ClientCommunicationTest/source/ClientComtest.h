#ifndef CLIENTCOMTEST_H
#define CLIENTCOMTEST_H

#include "ClientComtestCommon.h"

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class ClientComTest : public ClientComTestCommon
{
    Q_OBJECT

public:
    ClientComTest();

private slots:
    void sendMsgTest();
};

#endif // CLIENTCOMTEST_H
