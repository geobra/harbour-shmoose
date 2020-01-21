#include "DbusInterfaceWrapper.h"
#include <QDebug>

DbusInterfaceWrapper::DbusInterfaceWrapper(const QString &service, const QString &path, const QString &interface,
                                           const QDBusConnection &connection, QObject * const parent):
                                           QObject(const_cast<QObject *>(parent)), 
                                           interface_(new QDBusInterface(service, path, 
                                                                         interface, connection, 
                                                                         const_cast<QObject *>(parent)))
{
    interface_->setTimeout(1000);
    if(interface_->isValid() == false)
    {
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    }

    qDebug() << "QDbusConnection name: " << QDBusConnection::sessionBus().name() << ", env: " << qgetenv("DBUS_SESSION_BUS_ADDRESS");
}

void DbusInterfaceWrapper::callDbusMethodWithArgument(QString const &method, QList<QVariant> const &arguments,
                                                      QObject *receiver, const char *returnSlot,
                                                      const char *errorMethod)
{
    if (static_cast<QObject*>(NULL) == receiver)
    {
        receiver = this;
    }
    if (static_cast<char*>(NULL) == returnSlot)
    {
        returnSlot = SLOT(slotExpectTrueWithinTimeout(bool));
    }
    if (static_cast<char*>(NULL) == errorMethod)
    {
        errorMethod = SLOT(slotDbusError(QDBusError));
    }

    if (interface_->callWithCallback(method, arguments, receiver, returnSlot, errorMethod) == false)
    {
        qDebug() << "cant call dbus command: " + method + "on" + interface_->service();
    }
}

void DbusInterfaceWrapper::slotExpectTrueWithinTimeout(const bool status) const
{
    if (status == false)
    {
        qDebug() << "dbus command failed on " + interface_->service();
    }
}

void DbusInterfaceWrapper::slotDbusError(QDBusError error)
{
    qDebug() << error.message();
}

const QDBusInterface* DbusInterfaceWrapper::getInterface() const
{
    return interface_;
}

void DbusInterfaceWrapper::slotDaemonErrorMessage(QString value)
{
   qDebug() << "Daemon warning: " + value;
}

