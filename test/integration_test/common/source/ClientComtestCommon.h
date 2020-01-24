#ifndef CLIENTCOMTESTCOMMON_H
#define CLIENTCOMTESTCOMMON_H

#include <QObject>
#include <QtTest/QtTest>

class DbusInterfaceWrapper;

class ClientComTestCommon : public QObject
{
    Q_OBJECT

public:
    ClientComTestCommon();

public slots:
    void receiveConnectedSignal(QString str);

protected slots:
    void initTestCase();

private slots:
    void cleanupTestCase();

    void connectionTest();
    void requestRosterTest();
    void addContactTest();

protected:
    void connectionTestCommon(DbusInterfaceWrapper* interface, const QString& jid, const QString& pass);
    void requestRosterTestCommon(DbusInterfaceWrapper *interface);
    void addContactTestCommon(DbusInterfaceWrapper *interface, const QString& jid, const QString& name);

    void generatePicture();

    DbusInterfaceWrapper* interfaceLhs_;
    DbusInterfaceWrapper* interfaceRhs_;

    const QString user1jid_;
    const QString user2jid_;

    const QString imageFileName_;

    const int timeOut_;
};

#endif // CLIENTCOMTESTCOMMON_H
