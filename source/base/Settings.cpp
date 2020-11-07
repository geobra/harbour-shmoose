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

bool Settings::getDisplayChatNotifications() const
{
    bool save = true;

    QSettings settings;
    save = settings.value("notifications/displayChatNotifications", true).toBool();

    return save;
}

void Settings::setDisplayChatNotifications(bool DisplayChatNotifications)
{
    QSettings settings;
    settings.setValue("notifications/displayChatNotifications", DisplayChatNotifications);
    emit saveCredentialsChanged(DisplayChatNotifications);
}

bool Settings::getDisplayGroupchatNotifications() const
{
    bool save = true;

    QSettings settings;
    save = settings.value("notifications/displayGroupchatNotifications", false).toBool();

    return save;
}

void Settings::setDisplayGroupchatNotifications(bool DisplayGroupchatNotifications)
{
    QSettings settings;
    settings.setValue("notifications/displayGroupchatNotifications", DisplayGroupchatNotifications);
    emit saveCredentialsChanged(DisplayGroupchatNotifications);
}

QStringList Settings::getForceOnNotifications() const
{
    QSettings settings;
    QStringList jids;

    jids = settings.value("notifications/forceOnNotifications").toStringList();

    return jids;
}

void Settings::setForceOnNotifications(QStringList const & ForceOnNotifications)
{
    QSettings settings;

    if (ForceOnNotifications.length() > 0)
    {
        settings.setValue("notifications/forceOnNotifications", QVariant::fromValue(ForceOnNotifications));
    }
    else
    {
        settings.remove("notifications/forceOnNotifications");
    }

    emit forceOnNotificationsChanged(ForceOnNotifications);
}

void Settings::removeForceOnNotifications(QString const & Jid)
{
    QSettings settings;
    QStringList jids;

    if(settings.contains("notifications/forceOnNotifications"))
    {
        jids = settings.value("notifications/forceOnNotifications").toStringList();

        int idx = jids.indexOf(Jid);
        if(idx >= 0)
        {
            jids.removeAt(idx);
            setForceOnNotifications(jids);
        }
    }
}

void Settings::addForceOnNotifications(QString const & Jid)
{
    QSettings settings;
    QStringList jids;

    if(settings.contains("notifications/forceOnNotifications"))
    {
        jids = settings.value("notifications/forceOnNotifications").toStringList();
    }

    if(!jids.contains(Jid))
    {
        jids.append(Jid);
        setForceOnNotifications(jids);
    }
}

QStringList Settings::getForceOffNotifications() const
{
    QSettings settings;
    QStringList jids;

    jids = settings.value("notifications/forceOffNotifications").toStringList();

    return jids;
}

void Settings::setForceOffNotifications(QStringList const & ForceOffNotifications)
{
    QSettings settings;

    if (ForceOffNotifications.length() > 0)
    {
        settings.setValue("notifications/forceOffNotifications", QVariant::fromValue(ForceOffNotifications));
    }
    else
    {
        settings.remove("notifications/forceOffNotifications");
    }

    emit forceOffNotificationsChanged(ForceOffNotifications);
}

void Settings::removeForceOffNotifications(QString const & Jid)
{
    QSettings settings;
    QStringList jids;

    if(settings.contains("notifications/forceOffNotifications"))
    {
        jids = settings.value("notifications/forceOffNotifications").toStringList();

        int idx = jids.indexOf(Jid);
        if(idx >= 0)
        {
            jids.removeAt(idx);
            setForceOffNotifications(jids);
        }
    }
}

void Settings::addForceOffNotifications(QString const & Jid)
{
    QSettings settings;
    QStringList jids;

    if(settings.contains("notifications/forceOffNotifications"))
    {
        jids = settings.value("notifications/forceOffNotifications").toStringList();
    }

    if(!jids.contains(Jid))
    {
        jids.append(Jid);
        setForceOffNotifications(jids);
    }
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

void Settings::removeImagePath(QString const & Path)
{
    QSettings settings;
    QStringList searchPaths;

    if(settings.contains("storage/imagePaths"))
    {
        searchPaths = settings.value("storage/imagePaths").toStringList();

        int idx = searchPaths.indexOf(Path);
        if(idx >= 0)
        {
            searchPaths.removeAt(idx);
            setImagePaths(searchPaths);
        }
    }
}

void Settings::addImagePath(QUrl const & Path)
{
    QSettings settings;
    QStringList searchPaths;

    if(settings.contains("storage/imagePaths"))
    {
        searchPaths = settings.value("storage/imagePaths").toStringList();
    }

    if(!searchPaths.contains(Path.path()))
    {
        searchPaths.append(Path.path());

        setImagePaths(searchPaths);
    }
}


