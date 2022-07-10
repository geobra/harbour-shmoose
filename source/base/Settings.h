#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QUrl>
#include <QStringList>

class Settings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Jid READ getJid WRITE setJid NOTIFY jidChanged)
    Q_PROPERTY(QString Password READ getPassword WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool SaveCredentials READ getSaveCredentials WRITE setSaveCredentials NOTIFY saveCredentialsChanged)
    Q_PROPERTY(bool DisplayChatNotifications READ getDisplayChatNotifications WRITE setDisplayChatNotifications NOTIFY displayChatNotificationsChanged)
    Q_PROPERTY(bool DisplayGroupchatNotifications READ getDisplayGroupchatNotifications WRITE setDisplayGroupchatNotifications NOTIFY displayGroupchatNotificationsChanged)
    Q_PROPERTY(QStringList ForceOnNotifications READ getForceOnNotifications WRITE setForceOnNotifications NOTIFY forceOnNotificationsChanged)
    Q_PROPERTY(QStringList ForceOffNotifications READ getForceOffNotifications WRITE setForceOffNotifications NOTIFY forceOffNotificationsChanged)
    Q_PROPERTY(bool SendReadNotifications READ getSendReadNotifications WRITE setSendReadNotifications NOTIFY sendReadNotificationsChanged)
    Q_PROPERTY(QStringList ImagePaths READ getImagePaths WRITE setImagePaths NOTIFY imagePathsChanged)
    Q_PROPERTY(QStringList SendPlainText READ getSendPlainText WRITE setSendPlainText NOTIFY sendPlainTextChanged)
    Q_PROPERTY(bool CompressImages READ getCompressImages WRITE setCompressImages NOTIFY compressImagesChanged)
    Q_PROPERTY(bool SendOnlyImages READ getSendOnlyImages WRITE setSendOnlyImages NOTIFY sendOnlyImagesChanged)
    Q_PROPERTY(int LimitCompression READ getLimitCompression WRITE setLimitCompression NOTIFY limitCompressionChanged)
    Q_PROPERTY(bool EnableSoftwareFeatureOmemo READ getSoftwareFeatureOmemoEnabled WRITE setSoftwareFeatureOmemoEnabled NOTIFY softwareFeatureOmemoEnabledChanged);
    Q_PROPERTY(QString ResourceId READ getResourceId WRITE setResourceId NOTIFY resourceIdChanged);
    Q_PROPERTY(bool AskBeforeDownloading READ getAskBeforeDownloading WRITE setAskBeforeDownloading NOTIFY askBeforeDownloadingChanged);


public:
    explicit Settings(QObject *parent = nullptr);

    QString getJid() const;
    QString getPassword() const;
    bool getSaveCredentials() const;
    bool getDisplayChatNotifications() const;
    bool getDisplayGroupchatNotifications() const;
    QStringList getForceOnNotifications() const;
    QStringList getForceOffNotifications() const;
    bool getSendReadNotifications() const;
    QStringList getImagePaths();
    bool isOmemoForSendingOff();
    QStringList getSendPlainText() const;
    bool getCompressImages() const;
    bool getSendOnlyImages() const;
    unsigned int getLimitCompression() const;
    bool getSoftwareFeatureOmemoEnabled() const;
    QString getResourceId() const;
    bool getAskBeforeDownloading() const;

signals:
    void jidChanged(QString Jid);
    void passwordChanged(QString Password);
    void saveCredentialsChanged(bool SaveCredentials);
    void displayChatNotificationsChanged(bool DisplayChatNotifications);
    void displayGroupchatNotificationsChanged(bool DisplayGroupchatNotifications);
    void forceOnNotificationsChanged(QStringList const & ForceOnNotifications);
    void forceOffNotificationsChanged(QStringList const & ForceOffNotifications);
    void sendReadNotificationsChanged(bool SendReadNotifications);
    void imagePathsChanged(QStringList const & ImagePaths);
    void sendPlainTextChanged(const QStringList& sendPlainText);
    void compressImagesChanged(bool CompressImages);
    void sendOnlyImagesChanged(bool SendOnlyImages);
    void limitCompressionChanged(unsigned int LimitCompression);
    void softwareFeatureOmemoEnabledChanged(bool enableSoftwareFeatureOmemo);
    void resourceIdChanged(QString ResourceId);
    void askBeforeDownloadingChanged(bool askBeforeDownloading);

public slots:
    void setJid(QString Jid);
    void setPassword(QString Password);
    void setSaveCredentials(bool SaveCredentials);
    void setDisplayChatNotifications(bool DisplayChatNotifications);
    void setDisplayGroupchatNotifications(bool DisplayGroupchatNotifications);
    void setForceOnNotifications(QStringList const & ForceOnNotifications);
    void removeForceOnNotifications(QString const & Jid);
    void addForceOnNotifications(QString const & Jid);
    void setForceOffNotifications(QStringList const & ForceOffNotifications);
    void removeForceOffNotifications(QString const & Jid);
    void addForceOffNotifications(QString const & Jid);
    void setSendReadNotifications(bool SendReadNotifications);
    void setImagePaths(QStringList const & ImagePaths);
    void removeImagePath(QString const & Path);
    void addImagePath(QUrl const & Path);
    void setOmemoForSendingOff(bool omemoForSendingOff);

    void addForcePlainTextSending(const QString& jid);
    void removeForcePlainTextSending(const QString& jid);
    void setSendPlainText(const QStringList& sendPlainText);
    void setCompressImages(bool AlwaysCompressImages);
    void setSendOnlyImages(bool SendOnlyImages);
    void setLimitCompression(unsigned int LimitCompression);
    void setSoftwareFeatureOmemoEnabled(bool enableSoftwareFeatureOmemo);
    void setResourceId(QString ResourceId);
    void setAskBeforeDownloading(bool AskBeforeDownloading);
};

#endif // SETTINGS_H
