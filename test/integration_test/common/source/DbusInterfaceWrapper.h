#ifndef DBUS_INTERFACE_WRAPPER_HEADER
#define DBUS_INTERFACE_WRAPPER_HEADER

#include <QDBusInterface>

class DbusInterfaceWrapper: public QObject
{
Q_OBJECT
public:
     DbusInterfaceWrapper(const QString &service, const QString &path, const QString &interface = QString(),
                         const QDBusConnection &connection = QDBusConnection::sessionBus(), QObject * const parent = Q_NULLPTR);
private slots:
    void slotExpectTrueWithinTimeout(const bool status) const;

    static void slotDbusError(QDBusError error);

    static void slotDaemonErrorMessage(QString value);

public:

    void callDbusMethodWithArgument(QString const &method, const QList<QVariant> &arguments, QObject *receiver = static_cast<QObject*>(NULL),
                                    const char* returnSlot  = static_cast<char*>(NULL),
                                    const char * errorMethod = static_cast<char*>(NULL)) ;

    const QDBusInterface* getInterface() const;

private:
    QDBusInterface* interface_;

};

#endif // DBUS_INTERFACE_WRAPPER_HEADER
 
