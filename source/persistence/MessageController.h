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
	void markMessageReceived(QString const &id);
	void markMessageSent(QString const &id);

signals:
    void signalMessageReceived(QString id, QString jid, QString message);

public slots:

private:
	void generateRoleNames();
	virtual void setTable ( const QString &table_name );
    void markColumnOfId(QString const &column, QString const &id);
    bool isMessageIdInDatabase(QString const &id);

	void printSqlError();

	QHash<int, QByteArray> roles_;
	Database *database_;
};

#endif // MESSAGECONTROLLER_H
