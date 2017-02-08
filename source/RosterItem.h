#ifndef ROSTERITEM_H
#define ROSTERITEM_H

#include <QObject>
#include <QQmlEngine>

class RosterItem : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(Subscription subscription READ getSubscription WRITE setSubscription NOTIFY subscriptionChanged)

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
        AVAILABILITY_OONLINE,
        AVAILABILITY_AWAY,
        AVAILABILITY_FFC,
        AVAILABILITY_XA,
        AVAILABILITY_DND,
        AVAILABILITY_NONE
    };
    Q_ENUMS()

	explicit RosterItem(QObject *parent = 0);
	RosterItem(const QString& jid, const QString& name, const Subscription& subscription, QObject* parent = 0);

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

signals:
	void nameChanged();
	void jidChanged();
	void subscriptionChanged();
    void availabilityChanged();
    void statusChanged();

public slots:

private:
	QString jid_;
	QString name_;
	Subscription subscription_;
    Availability availability_;
    QString status_;
};

#endif // ROSTERITEM_H
