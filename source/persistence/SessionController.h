#ifndef SESSIONCONTROLLER_H
#define SESSIONCONTROLLER_H

#include <QSqlTableModel>

class Database;

class SessionController : public QSqlTableModel
{
    Q_OBJECT

public:
    explicit SessionController(QObject *parent = 0);
    SessionController(Database *db, QObject *parent = 0);

    bool setup();

    Q_INVOKABLE QVariant data(const QModelIndex &requestedIndex, int role=Qt::DisplayRole ) const;
    virtual QHash<int, QByteArray> roleNames() const;

    void updateSession(QString const &jid, QString const &lastMessage);
    void updateNumberOfUnreadMessages(QString const &jid, unsigned int unreadMessages);

    void setCurrentChatPartner(QString const &jid);

signals:

public slots:

private:
    void generateRoleNames();
    virtual void setTable ( const QString &table_name );

    unsigned int getNumberOfUnreadMessagesForJid(QString const &jid);

    void printSqlError();

    QHash<int, QByteArray> roles_;
    Database *database_;

    QString currentChatPartner_;
};

#endif // SESSIONONTROLLER_H
