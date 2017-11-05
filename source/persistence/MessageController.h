#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QSqlTableModel>

class Database;

class MessageController : public QSqlTableModel
{
    Q_OBJECT

public:
    explicit MessageController(QObject *parent = 0);
    MessageController(Database *db, QObject *parent = 0);

    bool setup();

    Q_INVOKABLE QVariant data(const QModelIndex &requestedIndex, int role=Qt::DisplayRole ) const;
    virtual QHash<int, QByteArray> roleNames() const;

    void setFilterOnJid(QString const &jidFiler);
    bool addMessage(bool isGroupMessage, const QString &id, QString const &jid, const QString &resource, QString const &message, const QString &type, unsigned int direction);

    void markMessageDisplayedConfirmed(QString const &id);
    void markMessageDisplayed(QString const &id);
    void markMessageReceived(QString const &id);
    void markMessageSent(QString const &id);

    QPair<QString, int> getNewestReceivedMessageIdAndStateOfJid(QString const &jid);

signals:
    void signalMessageReceived(QString id, QString jid, QString message);

public slots:

private:
    void generateRoleNames();
    virtual void setTable ( const QString &table_name );
    void setMessageStateOfId(QString const &id, const int state);
    bool isMessageIdInDatabase(QString const &id);

    void remarkMessageToReceivedForJidOfId(QString const &id);
    QString getJidOfMessageId(const QString &id);
    int getStateOfMessageId(QString const &id);

    void printSqlError();

    QHash<int, QByteArray> roles_;
    Database *database_;

    enum MessageDirection
    {
        MESSAGE_DIRECTION_OUTGOING = 0,
        MESSAGE_DIRECTION_INCOMING
    };

    enum MessageState
    {
        MESSAGE_STATE_DISPLAYED_CONFIRMED = -1,     // sent by me to other client to confirm I read the message
        MESSAGE_STATE_DEFAULT,                      // default after I sent a message
        MESSAGE_STATE_SENT,                         // session management confirmed message is received by server
        MESSAGE_STATE_RECEIVED,                     // other client confirmed that message is received in app
        MESSAGE_STATE_DISPLAYED                     // other client confirmed that message is read (xep 0333, chat markers)
    };

};

#endif // MESSAGECONTROLLER_H
