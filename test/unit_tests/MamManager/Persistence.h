#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>

class Persistence
{
public:
    Persistence();

    void addMessage(const QString &id, QString const &jid, QString const &resource, QString const &message, const QString &type, unsigned int direction, qint64 timestamp = 0);
    void markGroupMessageReceivedByMember(const QString &msgId, const QString &resource);
    void markMessageAsReceivedById(const QString &msgId);

    void markGroupMessageDisplayedByMember(const QString &msgId, const QString &resource);
    void markMessageAsDisplayedId(const QString &msgId);

    void clear();

    QString id_;
    QString jid_;
    QString resource_;
    QString message_;
    QString type_;
    unsigned int direction_;
    qint64 timestamp_;

};

#endif // PERSISTENCE_H
