#include "Settings.h"

#include <QSettings>
#include <QString>

Settings::Settings(QObject *parent) : QObject(parent)
{

}


QString Settings::getJid() const
{
    QString returnValue = "";

    QSettings settings;
    if(settings.value("authentication/jid").toString() != "NOT_SET")
    {
        returnValue = settings.value("authentication/jid").toString();
    }

    return returnValue;
}

void Settings::setJid(QString Jid)
{
    QSettings settings;
    settings.setValue("authentication/jid", Jid);
    emit jidChanged(Jid);
}

QString Settings::getPassword() const
{
    QString returnValue = "";

    QSettings settings;
    if(settings.value("authentication/password").toString() != "NOT_SET")
    {
        returnValue = settings.value("authentication/password").toString();
    }

    return returnValue;
}

void Settings::setPassword(QString Password)
{
    QSettings settings;
    settings.setValue("authentication/password", Password);
    emit passwordChanged(Password);
}

bool Settings::getSaveCredentials() const
{
    bool save = false;

    QSettings settings;
    save = settings.value("authentication/saveCredentials", false).toBool();

    return save;
}

void Settings::setSaveCredentials(bool SaveCredentials)
{
    QSettings settings;
    settings.setValue("authentication/saveCredentials", SaveCredentials);
    emit saveCredentialsChanged(SaveCredentials);
}

bool Settings::getSendReadNotifications() const
{
    bool save = true;

    QSettings settings;
    save = settings.value("privacy/sendReadNotifications", true).toBool();

    return save;
}

void Settings::setSendReadNotifications(bool SendReadNotifications)
{
    QSettings settings;
    settings.setValue("privacy/sendReadNotifications", SendReadNotifications);
    emit saveCredentialsChanged(SendReadNotifications);
}


