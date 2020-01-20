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

    void generatePicture();

    DbusInterfaceWrapper* interfaceLhs_;
    DbusInterfaceWrapper* interfaceRhs_;

    const QString user1jid_;
    const QString user2jid_;

    const QString imageFileName_;

public slots:
    void receiveConnectedSignal(QString str);
};

#endif // CLIENTCOMTEST_H
