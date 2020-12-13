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

public:
    explicit Settings(QObject *parent = 0);

    QString getJid() const;
    QString getPassword() const;
    bool getSaveCredentials() const;
    bool getDisplayChatNotifications() const;
    bool getDisplayGroupchatNotifications() const;
    QStringList getForceOnNotifications() const;
    QStringList getForceOffNotifications() const;
    bool getSendReadNotifications() const;
    QStringList getImagePaths();
    bool isOmemoInitialized();

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
    void setOmemoInitialized(bool isInitialized);

};

#endif // SETTINGS_H
