#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>
#include <QPair>

class Persistence
{
public:
    Persistence();

    void addMessage(const QString &id, QString const &jid, QString const &resource, QString const &message,
                    const QString &type, unsigned int direction, unsigned int security, qint64 timestamp = 0);
    void markGroupMessageReceivedByMember(const QString &msgId, const QString &resource);
    void markMessageAsReceivedById(const QString &msgId);

    void markGroupMessageDisplayedByMember(const QString &msgId, const QString &resource);
    void markMessageAsDisplayedId(const QString &msgId);

    QPair<QString, int> getNewestReceivedMessageIdAndStateOfJid(QString const &jid)
    {
        return QPair<QString, int>("",0);
    }

    QString getResourceForMsgId(const QString& msgId)
    {
        return "abcdefg";
    }

    void markMessageDisplayedConfirmedId(QString const &id)
    {}

    void markMessageAsSentById(QString const &id)
    {}

    QString getCurrentChatPartner()
    {
        return "";
    }

    void clear();

    QString id_;
    QString jid_;
    QString resource_;
    QString message_;
    QString type_;
    unsigned int direction_;
    unsigned int security_;
    qint64 timestamp_;

    QString idDisplayed_{};
    QString resourceDisplayed_;

};

#endif // PERSISTENCE_H
