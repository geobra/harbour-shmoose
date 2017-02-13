#ifndef ROSTERCONTROLLER_H
#define ROSTERCONTROLLER_H

#include "RosterItem.h"

#include <Swiften/Swiften.h>
#include <QObject>
#include <QQmlListProperty>

class RosterController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<RosterItem> rosterList READ getRosterList NOTIFY rosterListChanged)

public:
	RosterController(QObject *parent = 0);

    void setClient(Swift::Client *client);

    Q_INVOKABLE void addContact(const QString& jid, const QString& name);
    Q_INVOKABLE void removeContact(const QString& jid);

	void requestRosterFromClient(Swift::Client *client);
	QQmlListProperty<RosterItem> getRosterList();

    void handleUpdateFromPresence(const Swift::JID &jid, const QString &status, const RosterItem::Availability& availability);

signals:
	void rosterListChanged();

public slots:

private:
	void handleRosterReceived(Swift::ErrorPayload::ref error);
    void handleJidAdded(const Swift::JID &jid);
    void handleJidUpdated(const Swift::JID &jid, const std::string &name, const std::vector< std::string > &);
    void handleJidRemoved(const Swift::JID &jid);

    void bindJidUpdateMethodes();

	Swift::Client* client_;
	QList<RosterItem*> rosterList_;
};

#endif // ROSTERCONTROLLER_H
