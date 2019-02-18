#ifndef ROSTERCONTROLLER_H
#define ROSTERCONTROLLER_H

#include "RosterItem.h"

#include <Swiften/Swiften.h>
#include <QObject>
#include <QQmlListProperty>

class PresenceHandler;

class RosterController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<RosterItem> rosterList READ getRosterList NOTIFY rosterListChanged)

    enum itemAttribute
    {
        attributePicturePath,
        attributeName
    };

public:
    RosterController(QObject *parent = 0);

    void setupWithClient(Swift::Client *client);

    Q_INVOKABLE void addContact(const QString& jid, const QString& name);
    Q_INVOKABLE void removeContact(const QString& jid);
    Q_INVOKABLE bool isGroup(QString const &jid);

    Q_INVOKABLE QString getAvatarImagePathForJid(QString const &jid);
    Q_INVOKABLE QString getNameForJid(QString const &jid);

    void requestRosterFromClient(Swift::Client *client);
    QQmlListProperty<RosterItem> getRosterList();

    void handleUpdateFromPresence(const Swift::JID &jid, const QString &status, const RosterItem::Availability& availability);

    void updateNameForJid(const Swift::JID &jid, const std::string &name);

signals:
    void rosterListChanged();
    void signalShowMessage(QString headline, QString body);

public slots:
    void addGroupAsContact(QString groupJid, QString groupName);
    void removeGroupFromContacts(QString groupJid);

private:
    void handleRosterReceived(Swift::ErrorPayload::ref error);
    void handleJidAdded(const Swift::JID &jid);
    void handleJidUpdated(const Swift::JID &jid, const std::string &name, const std::vector< std::string > &);
    void handleJidRemoved(const Swift::JID &jid);

    void sendUnavailableAndUnsubscribeToJid(const QString& jid);

    void handleVCardChanged(const Swift::JID &jid, const Swift::VCard::ref &vCard);

    void handleMessageReceived(Swift::Message::ref message);

    void bindJidUpdateMethodes();

    bool checkHashDiffers(QString const &jid, QString const &newHash);
    void sortRosterList();

    QString getTypeForJid(itemAttribute const &attribute, QString const &jid);

    Swift::Client* client_;
    QList<RosterItem*> rosterList_;

    PresenceHandler* presenceHandler_;
};

#endif // ROSTERCONTROLLER_H
