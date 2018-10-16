#include "Settings.h"

#include <QSettings>
#include <QString>
#include <QStringList>
#include <QStandardPaths>

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

QStringList Settings::getImagePaths()
{
    QSettings settings;
    QStringList searchPaths;

    if(!settings.contains("storage/imagePaths"))
    {
        searchPaths.append(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        setImagePaths(searchPaths);
    }
    else
    {
        searchPaths = settings.value("storage/imagePaths").toStringList();
    }

    return searchPaths;
}

void Settings::setImagePaths(QStringList const & ImagePaths)
{
    QSettings settings;

    settings.setValue("storage/imagePaths", QVariant::fromValue(ImagePaths));

    emit imagePathsChanged(ImagePaths);
}





