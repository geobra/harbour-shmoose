#ifndef ROSTERITEM_H
#define ROSTERITEM_H

#include <QObject>
#include <QQmlEngine>
#include <QImage>

class RosterItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
    Q_PROPERTY(Subscription subscription READ getSubscription WRITE setSubscription NOTIFY subscriptionChanged)
    Q_PROPERTY(Availability availability READ getAvailability WRITE setAvailability NOTIFY availabilityChanged)
    Q_PROPERTY(QString status READ getStatus WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString imagePath READ getImagePath NOTIFY imageChanged)
    Q_PROPERTY(QString isGroup READ isGroup NOTIFY isGroupChanged)

public:

    enum Subscription
    {
        SUBSCRIPTION_NONE,
        SUBSCRIPTION_TO,
        SUBSCRIPTION_FROM,
        SUBSCRIPTION_BOTH
    };
    Q_ENUMS(Subscription)

    enum Availability
    {
        AVAILABILITY_UNKNOWN,
        AVAILABILITY_ONLINE,
        AVAILABILITY_OFFLINE
    };
    Q_ENUMS(Availability)

    explicit RosterItem(QObject *parent = 0);
    RosterItem(const QString& jid, const QString& name, const Subscription& subscription, bool isGroup, QObject* parent = 0);

    QString getName();
    void setName(const QString& name);

    QString getJid();
    void setJid(const QString& jid);

    Subscription getSubscription();
    void setSubscription(const Subscription& subscription);

    Availability getAvailability();
    void setAvailability(const Availability& availability);

    QString getStatus();
    void setStatus(const QString& status);

    QString getImagePath();
    void triggerNewImage();

    bool isGroup();

signals:
    void nameChanged();
    void jidChanged();
    void subscriptionChanged();
    void availabilityChanged();
    void statusChanged();
    void isGroupChanged();
    void imageChanged();

public slots:

private:
    QString jid_;
    QString name_;
    Subscription subscription_;
    Availability availability_;
    QString status_;
    bool isGroup_;
};

#endif // ROSTERITEM_H
