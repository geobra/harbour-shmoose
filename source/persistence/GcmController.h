#ifndef GCMCONTROLLER_H
#define GCMCONTROLLER_H

#include <QSqlTableModel>

class Database;

class GcmController : public QSqlTableModel
{
	Q_OBJECT

public:
	explicit GcmController(QObject *parent = 0);
	GcmController(Database *db, QObject *parent = 0);

	bool setup();

	Q_INVOKABLE QVariant data(const QModelIndex &requestedIndex, int role=Qt::DisplayRole ) const;
	Q_INVOKABLE void setFilterOnMsg(QString const &msg);

	virtual QHash<int, QByteArray> roleNames() const;

	void markGroupMessageReceivedByMember(const QString& msgId, const QString& groupChatMember);
	void markGroupMessageDisplayedByMember(const QString& msgId, const QString& groupChatMember);

    const QString getResourcesOfNewestDisplayedMsgforJid(const QString& jid);

signals:
    void signalRoomMessageStateChanged(QString, QString, int);

public slots:

private:
	void generateRoleNames();
	virtual void setTable ( const QString &table_name );

	void markGroupMessage(const QString& msgId, const QString& groupChatMember, int state);

	int getMsgStatus(const QString& msgId, const QString &groupChatMember);

    QString getNewestMessageIdOfJid(const QString& jid);

	void printSqlError();

	QHash<int, QByteArray> roles_;
	Database *database_;
};

#endif // GCMCONTROLLER_H
