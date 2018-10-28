#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Jid READ getJid WRITE setJid NOTIFY jidChanged)
    Q_PROPERTY(QString Password READ getPassword WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool SaveCredentials READ getSaveCredentials WRITE setSaveCredentials NOTIFY saveCredentialsChanged)
    Q_PROPERTY(bool SendReadNotifications READ getSendReadNotifications WRITE setSendReadNotifications NOTIFY sendReadNotificationsChanged)

    QString m_SendReadNotifications;

public:
    explicit Settings(QObject *parent = 0);

    QString getJid() const;
    QString getPassword() const;
    bool getSaveCredentials() const;
    bool getSendReadNotifications() const;

signals:
    void jidChanged(QString Jid);
    void passwordChanged(QString Password);
    void saveCredentialsChanged(bool SaveCredentials);
    void sendReadNotificationsChanged(bool SendReadNotifications);

public slots:
    void setJid(QString Jid);
    void setPassword(QString Password);
    void setSaveCredentials(bool SaveCredentials);
    void setSendReadNotifications(bool SendReadNotifications);

};

#endif // SETTINGS_H
