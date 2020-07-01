#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>

class Persistence
{
public:
    Persistence();

    void addMessage(const QString &id, QString const &jid, QString const &resource, QString const &message, const QString &type, unsigned int direction);
    void clear();

    QString id_;
    QString jid_;
    QString resource_;
    QString message_;
    QString type_;
    unsigned int direction_;

};

#endif // PERSISTENCE_H
