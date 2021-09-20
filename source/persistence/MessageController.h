#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QSqlTableModel>

class Database;

class MessageController : public QSqlTableModel
{
    Q_OBJECT

public:
    explicit MessageController(QObject *parent = nullptr);
    MessageController(Database *db, QObject *parent = nullptr);

    bool setup();

    Q_INVOKABLE QVariant data(const QModelIndex &requestedIndex, int role=Qt::DisplayRole ) const;
    virtual QHash<int, QByteArray> roleNames() const;

    void setFilterOnJid(QString const &jidFiler);
    bool addMessage(const QString &id, QString const &jid, const QString &resource, QString const &message,
                    const QString &type, unsigned int direction, unsigned int security, qint64 timestamp = 0);
    bool removeMessage(const QString& id, const QString &jid);

    void markMessageDisplayedConfirmed(QString const &id);
    void markMessageDisplayed(QString const &id);
    void markMessageReceived(QString const &id);
    void markMessageSent(QString const &id);
    void markMessageUploadingAttachment(QString const &id);
    void markMessageSendFailed(QString const &id);

    QPair<QString, int> getNewestReceivedMessageIdAndStateOfJid(QString const &jid);
    QString getRessourceForMsgId(const QString& msgId);
    void removeMessagesFromJid(const QString& jid);

signals:
    void signalMessageReceived(QString id, QString jid, QString message);
    void signalMessageStateChanged(QString msgId, int state);

public slots:

private:
    void generateRoleNames();
    virtual void setTable ( const QString &table_name );
    void setMessageStateOfId(QString const &id, const int state);
    bool isMessageIdForJidInDatabase(QString const &id, const QString &jid);

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
        MESSAGE_STATE_DISPLAYED,                    // other client confirmed that message is read (xep 0333, chat markers)
        MESSAGE_STATE_UPLOADING_ATTACHMENT,         // Uploading attachment in progress
        MESSAGE_STATE_SEND_FAILED                   // Message recorded in database but not sent
    };

};

#endif // MESSAGECONTROLLER_H