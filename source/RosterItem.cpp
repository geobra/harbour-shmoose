#include "RosterItem.h"
#include "System.h"

#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

RosterItem::RosterItem(QObject *parent) : QObject(parent), jid_(""), name_(""), subscription_(SUBSCRIPTION_NONE), availability_(AVAILABILITY_UNKNOWN), status_("")
{
}

RosterItem::RosterItem(const QString &jid, const QString &name, const Subscription &subscription, bool isGroup, QObject* parent) :
    QObject(parent), jid_(jid), name_(name), subscription_(subscription), availability_(AVAILABILITY_UNKNOWN), status_(""), isGroup_(isGroup)
{
}

QString RosterItem::getName()
{
    QString returnName = name_;

    if (returnName.isEmpty())
    {
        QStringList nameAndDomain = jid_.split( "@" );
        if (nameAndDomain.size() == 2)
        {
            returnName = nameAndDomain.at(0);
        }
    }

    return returnName;
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
    qDebug() << "RosterItem::setAvailability" << availability;
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

QString RosterItem::getImagePath()
{
    QString imagePath = "";

    QString avatarFile = getJid();
    avatarFile.replace("@","-at-");

    QString avatarImagePath = System::getAvatarPath() + QDir::separator() + avatarFile + ".png";

    QFileInfo info(avatarImagePath);

    if (info.exists() && info.isFile())
    {
        imagePath = avatarImagePath;
    }

    return imagePath;
}

void RosterItem::triggerNewImage()
{
    // triggerd if a new image is successfull written to disk
    emit imageChanged();
}

bool RosterItem::isGroup()
{
    return isGroup_;
}
