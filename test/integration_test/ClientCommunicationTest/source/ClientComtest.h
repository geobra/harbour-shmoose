#ifndef CLIENTCOMTEST_H
#define CLIENTCOMTEST_H

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class ClientComTest : public QObject
{
    Q_OBJECT

public:
    ClientComTest();

private slots:
    void initTestCase();
    void cleanupTestCase();

    void connectionTest();
    void requestRosterTest();
    void addContactTest();    

    void sendMsgTest();

    void quitClientsTest();

private:
    void connectionTestCommon(DbusInterfaceWrapper* interface, const QString& jid, const QString& pass);
    void requestRosterTestCommon(DbusInterfaceWrapper *interface);
    void addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name);

    DbusInterfaceWrapper* interfaceLhs;
    DbusInterfaceWrapper* interfaceRhs;

    const QString user1jid;
    const QString user2jid;

public slots:
    void receiveConnectedSignal(QString str);
};

#endif // CLIENTCOMTEST_H
