#include "RosterItem.h"

RosterItem::RosterItem(QObject *parent) : QObject(parent), jid_(""), name_(""), subscription_(SUBSCRIPTION_NONE)
{
}

RosterItem::RosterItem(const QString &jid, const QString &name, const Subscription &subscription, QObject* parent) :
    QObject(parent), jid_(jid), name_(name), subscription_(subscription), availability_(AVAILABILITY_NONE), status_("")
{
}

QString RosterItem::getName()
{
	return name_;
}

void RosterItem::setName(const QString &name)
{
	name_ = name;

	emit nameChanged();
}

QString RosterItem::getJid()
{
	return jid_;
}

void RosterItem::setJid(const QString &jid)
{
	jid_ = jid;

	emit jidChanged();
}

RosterItem::Subscription RosterItem::getSubscription()
{
	return subscription_;
}

void RosterItem::setSubscription(const Subscription &subscription)
{
	subscription_ = subscription;

	emit subscriptionChanged();
}

RosterItem::Availability RosterItem::getAvailability()
{
    return availability_;
}

void RosterItem::setAvailability(const Availability& availability)
{
    availability_ = availability;

    emit availabilityChanged();
}

QString RosterItem::getStatus()
{
    return status_;
}

void RosterItem::setStatus(const QString &status)
{
    status_ = status;

    emit statusChanged();
}
