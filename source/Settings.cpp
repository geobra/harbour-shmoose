#include "Settings.h"

#include <QSettings>

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

//QStringList Settings::getImagePaths() const;
//void Settings::setImagePaths(QStringList ImagePaths);





