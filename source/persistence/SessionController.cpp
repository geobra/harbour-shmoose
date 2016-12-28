#include "SessionController.h"
#include "Database.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>
#include <QSqlQuery>

#include <QDebug>

SessionController::SessionController(QObject *parent) : QSqlTableModel(parent)
{
}

SessionController::SessionController(Database *db, QObject *parent) :
	QSqlTableModel(parent, *(db->getPointer())), database_(db), currentChatPartner_("")
{
	setEditStrategy(QSqlTableModel::OnRowChange);
	setTable("sessions");
	setSort(2, Qt::DescendingOrder);
	if (!select())
	{
		qDebug() << "error on select in SessionController::SessionController";
	}
}

QVariant SessionController::data ( const QModelIndex & requestedIndex, int role ) const
{
	if(requestedIndex.row() >= rowCount())
	{
		return QString("");
	}

	if(role < Qt::UserRole)
	{
		QVariant data = QSqlTableModel::data(requestedIndex, role);
		return data;
	}

	QModelIndex modelIndex = this->index(requestedIndex.row(), role - Qt::UserRole - 1 );
	QVariant editData = QSqlQueryModel::data(modelIndex, Qt::EditRole);

	return editData;
}

// Role names are set to whatever your db table's column names are.
void SessionController::generateRoleNames()
{
	roles_.clear();
	for (int i = 0; i < columnCount(); i++)
	{
		roles_[Qt::UserRole + i + 1] = QVariant(headerData(i, Qt::Horizontal).toString()).toByteArray();
	}
}

QHash<int, QByteArray> SessionController::roleNames() const
{
	return roles_;
}

void SessionController::setTable ( const QString &table_name )
{
	QSqlTableModel::setTable(table_name);
	generateRoleNames();
}

int SessionController::getRowNumberForJid(QString const &jid)
{
	int returnValue = -1;

	for (int row = 0; row < rowCount(); row++)
	{
		QString jidInModel = record(row).value("jid").toString();

		if (jidInModel == jid)
		{
			returnValue = row;
		}
	}

	return returnValue;
}

unsigned int SessionController::getNumberOfUnreadMessagesInRow(unsigned int row)
{
	unsigned int returnValue = 0;

	if (row < static_cast<unsigned int>(this->rowCount()))
	{
		returnValue = record(row).value("unreadmessages").toInt();
	}

	return returnValue;
}

void SessionController::updateNumberOfUnreadMessages(QString const &jid, unsigned int unreadMessages)
{
	int row = getRowNumberForJid(jid);

	if (row >= 0) // no need to update, if not already in db
	{
		QSqlRecord record = this->record(row);
		record.setValue("unreadmessages", unreadMessages);

		if (this->setRecord(row, record) == false)
		{
			printSqlError();
		}
		else
		{
			if (! this->submitAll())
			{
				printSqlError();
			}
		}

		// update the model with the changes of the database
		if (select() != true)
		{
			qDebug() << "error on select in MessageController::addMessage";
		}
	}
}

void SessionController::updateSession(QString const &jid, QString const &lastMessage)
{
	int row = getRowNumberForJid(jid);

	QSqlRecord record;
	if (row == -1)
	{
		record = this->record();
	}
	else
	{
		record = this->record(row);
	}

	unsigned int unreadMessages = getNumberOfUnreadMessagesInRow(row);
	record.setValue("jid", jid);
	record.setValue("lastmessage", lastMessage);
	record.setValue("timestamp", QDateTime::currentDateTime().toTime_t() );
	if (jid != currentChatPartner_)
	{
		record.setValue("unreadmessages", ++unreadMessages);
	}

	bool submitRecord = false;
	if (row == -1)
	{
		submitRecord = this->insertRecord(-1, record);
	}
	else
	{
		submitRecord = this->setRecord(row, record);
	}

	if (submitRecord == false)
	{
		printSqlError();
	}
	else
	{
		if (! this->submitAll())
		{
			printSqlError();
		}
	}

	// update the model with the changes of the database
	if (select() != true)
	{
		qDebug() << "error on select in MessageController::addMessage";
	}

	database_->dumpDataToStdOut();
}

void SessionController::setCurrentChatPartner(QString const &jid)
{
	currentChatPartner_ = jid;
}

void SessionController::printSqlError()
{
	qDebug() << this->lastError().databaseText();
	qDebug() << this->lastError().driverText();
	qDebug() << this->lastError().text();
}
