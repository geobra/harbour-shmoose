#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString Jid READ getJid WRITE setJid NOTIFY jidChanged)
    Q_PROPERTY(QString Password READ getPassword WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool SaveCredentials READ getSaveCredentials WRITE setSaveCredentials NOTIFY saveCredentialsChanged)
    Q_PROPERTY(QStringList ImagePaths READ getImagePaths WRITE setImagePaths NOTIFY imagePathsChanged)

public:
    explicit Settings(QObject *parent = 0);

    QString getJid() const;
    QString getPassword() const;
    bool getSaveCredentials() const;
    QStringList getImagePaths();

signals:
    void jidChanged(QString Jid);
    void passwordChanged(QString Password);
    void saveCredentialsChanged(bool SaveCredentials);
    void imagePathsChanged(QStringList const & ImagePaths);

public slots:
    void setJid(QString Jid);
    void setPassword(QString Password);
    void setSaveCredentials(bool SaveCredentials);
    void setImagePaths(QStringList const & ImagePaths);
};

#endif // SETTINGS_H
