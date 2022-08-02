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

void Settings::addForcePlainTextSending(const QString& jid)
{
    QSettings settings;
    QStringList jids;

    if(settings.contains("omeo/sendPlainText"))
    {
        jids = settings.value("omeo/sendPlainText").toStringList();
    }

    if(!jids.contains(jid))
    {
        jids.append(jid);
        setSendPlainText(jids);
    }
}

void Settings::removeForcePlainTextSending(const QString& jid)
{
    QSettings settings;
    QStringList jids;

    if(settings.contains("omeo/sendPlainText"))
    {
        jids = settings.value("omeo/sendPlainText").toStringList();

        int idx = jids.indexOf(jid);
        if(idx >= 0)
        {
            jids.removeAt(idx);
            setSendPlainText(jids);
        }
    }
}

void Settings::setSendPlainText(const QStringList& sendPlainText)
{
    QSettings settings;

    if (sendPlainText.length() > 0)
    {
        settings.setValue("omeo/sendPlainText", QVariant::fromValue(sendPlainText));
    }
    else
    {
        settings.remove("omeo/sendPlainText");
    }

    emit sendPlainTextChanged(sendPlainText);
}

QStringList Settings::getSendPlainText() const
{
    QSettings settings;
    return settings.value("omeo/sendPlainText").toStringList();
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

void Settings::setOmemoForSendingOff(bool omemoForSendingOff)
{
    QSettings settings;
    settings.setValue("omemo/omemoForSendingOff", omemoForSendingOff);
}

bool Settings::isOmemoForSendingOff()
{
    QSettings settings;
    return settings.value("omemo/omemoForSendingOff", false).toBool();
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

bool Settings::getCompressImages() const
{
    bool save;

    QSettings settings;
    save = settings.value("attachments/compressImages", false).toBool();

    return save;
}

void Settings::setCompressImages(bool CompressImages)
{
    QSettings settings;
    settings.setValue("attachments/compressImages", CompressImages);
    emit compressImagesChanged(CompressImages);
}

bool Settings::getSendOnlyImages() const
{
    bool save;

    QSettings settings;
    save = settings.value("attachments/sendOnlyImages", true).toBool();

    return save;
}

void Settings::setSendOnlyImages(bool SendOnlyImages)
{
    QSettings settings;
    settings.setValue("attachments/sendOnlyImages", SendOnlyImages);
    emit sendOnlyImagesChanged(SendOnlyImages);
}

unsigned int Settings::getLimitCompression() const
{
    unsigned int save;

    QSettings settings;
    save = settings.value("attachments/limitCompression", 400000u).toUInt();

    return save;
}

void Settings::setLimitCompression(unsigned int LimitCompression)
{
    QSettings settings;
    settings.setValue("attachments/limitCompression", LimitCompression);
    emit limitCompressionChanged(LimitCompression);
}

bool Settings::getSoftwareFeatureOmemoEnabled() const
{
    bool enabled{false};

    QSettings settings;
    enabled = settings.value("swfeatures/omemo", false).toBool();

    return enabled;
}

void Settings::setSoftwareFeatureOmemoEnabled(bool enableSoftwareFeatureOmemo)
{
    QSettings settings;
    settings.setValue("swfeatures/omemo", enableSoftwareFeatureOmemo);
    emit softwareFeatureOmemoEnabledChanged(enableSoftwareFeatureOmemo);
}

QString Settings::getResourceId() const
{
   QString returnValue = "";

    QSettings settings;
    if(settings.value("authentication/resourceId").toString() != "NOT_SET")
    {
        returnValue = settings.value("authentication/resourceId").toString();
    }

    return returnValue;
}

void Settings::setResourceId(QString resourceId)
{
    QSettings settings;
    settings.setValue("authentication/resourceId", resourceId);
    emit resourceIdChanged(resourceId);
}

bool Settings::getAskBeforeDownloading() const
{
    bool enabled{false};

    QSettings settings;
    enabled = settings.value("attachments/askBeforeDownloading", false).toBool();

    return enabled;
}

void Settings::setAskBeforeDownloading(bool askBeforeDownloading)
{
    QSettings settings;
    settings.setValue("attachments/askBeforeDownloading", askBeforeDownloading);
    emit askBeforeDownloadingChanged(askBeforeDownloading);
}