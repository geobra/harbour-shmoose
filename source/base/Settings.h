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
    Q_PROPERTY(bool SendReadNotifications READ getSendReadNotifications WRITE setSendReadNotifications NOTIFY sendReadNotificationsChanged)
    Q_PROPERTY(QStringList ImagePaths READ getImagePaths WRITE setImagePaths NOTIFY imagePathsChanged)

public:
    explicit Settings(QObject *parent = 0);

    QString getJid() const;
    QString getPassword() const;
    bool getSaveCredentials() const;
    bool getSendReadNotifications() const;
    QStringList getImagePaths();

signals:
    void jidChanged(QString Jid);
    void passwordChanged(QString Password);
    void saveCredentialsChanged(bool SaveCredentials);
    void sendReadNotificationsChanged(bool SendReadNotifications);
    void imagePathsChanged(QStringList const & ImagePaths);

public slots:
    void setJid(QString Jid);
    void setPassword(QString Password);
    void setSaveCredentials(bool SaveCredentials);
    void setSendReadNotifications(bool SendReadNotifications);
    void setImagePaths(QStringList const & ImagePaths);
    void removeImagePath(QString const & Path);
    void addImagePath(QUrl const & Path);

};

#endif // SETTINGS_H
