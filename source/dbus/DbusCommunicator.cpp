#include "DbusCommunicator.h"

#include "Shmoose.h"
#include "RosterController.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusError>

DbusCommunicator::DbusCommunicator(const QString &path, const QString &name, QObject * const parent) : QObject(parent), dbusObjectPath_(path), dbusServiceName_(name)
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        qDebug() << "Cannot connect to the D-Bus session bus.";
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerService(dbusServiceName_))
    {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerObject(dbusObjectPath_, dbusServiceName_, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignal))
    {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }
}

void DbusCommunicator::setXmpClient(Shmoose* shmoose)
{
    shmoose_ = shmoose;
}

bool DbusCommunicator::tryToConnect(const QString& jid, const QString& pass)
{
    qDebug() << "try to connect as " << jid << " with pass " << pass;

    connect(shmoose_, SIGNAL(connectionStateChanged()), this, SLOT(clientConnected()));
    shmoose_->mainConnect(jid, pass);

    return true;
}

void DbusCommunicator::clientConnected()
{
    qDebug() << "connected!";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "connected");
    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

bool DbusCommunicator::requestRoster()
{
    qDebug() << "requestRoster";
    RosterController* rc = shmoose_->getRosterController();
    rc->requestRoster();

    connect(rc, SIGNAL(rosterListChanged()), this, SLOT(gotRosterEntry()));

    return true;
}

bool DbusCommunicator::addContact(const QString& jid, const QString& name)
{
    qDebug() << "addContact: jid: " << jid << ", name: " << name;
    RosterController* rc = shmoose_->getRosterController();
    rc->addContact(jid, name);

    connect(rc, SIGNAL(rosterListChanged()), this, SLOT(gotRosterEntry()));

    return true;
}

void DbusCommunicator::gotRosterEntry()
{
    qDebug() << "newRosterEntry!";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "newRosterEntry");
    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

bool DbusCommunicator::sendMsg(const QString& jid, const QString& msg)
{
    qDebug() << "sendMsg";

    Persistence *persistence = shmoose_->getPersistence();
    connect(persistence, SIGNAL(messageControllerChanged()), this, SLOT(msgChanged()));

    shmoose_->sendMessage(jid, msg, "msg");

    return true;
}

void DbusCommunicator::msgChanged()
{
    qDebug() << "msgChanged";

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalMsgChanged");
    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }
}

bool DbusCommunicator::requestLatestMsgForJid(const QString& jid)
{
    qDebug() << "requestLatestMsgForJid: " << jid;
    Persistence *persistence = shmoose_->getPersistence();
    QPair<QString, int> msgAndState = persistence->getNewestReceivedMessageIdAndStateOfJid(jid);

    //qDebug() << "found msg: " << msgAndState.first << ", state: " << msgAndState.second;

    QDBusMessage msg = QDBusMessage::createSignal(dbusObjectPath_, dbusServiceName_, "signalLatestMsg");
    msg << msgAndState.first;
    msg << msgAndState.second;

    if(QDBusConnection::sessionBus().send(msg) == false)
    {
        qDebug() << "cant send message via dbus";
    }

    return true;
}
