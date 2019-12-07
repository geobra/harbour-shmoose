#ifndef CLIENTCOMTEST_H
#define CLIENTCOMTEST_H

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class ClientComTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void connectionTest();
    void requestRosterTest();
    void addContactTest();    

    void sendMsgTest();

private:
    void connectionTestCommon(DbusInterfaceWrapper* interface, const QString& jid, const QString& pass);
    void requestRosterTestCommon(DbusInterfaceWrapper *interface);
    void addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name);

    DbusInterfaceWrapper* interfaceLhs;
    DbusInterfaceWrapper* interfaceRhs;

public slots:
    void receiveConnectedSignal(QString str);
};

#endif // CLIENTCOMTEST_H
