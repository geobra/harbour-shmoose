#include "MessageController.h"
#include "Database.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>

#include <QDebug>

MessageController::MessageController(QObject *parent) : QSqlTableModel(parent)
{
}

MessageController::MessageController(Database *db, QObject *parent) :
	QSqlTableModel(parent, *(db->getPointer())), database_(db)
{
	setEditStrategy(QSqlTableModel::OnRowChange);
	setTable("messages");
    setSort(4, Qt::AscendingOrder); // 4 -> timestamp
	if (!select())
	{
		qDebug() << "error on select in MessageController::MessageController";
	}
}

void MessageController::setFilterOnJid(QString const &jidFiler)
{
	setFilter("jid = '" + jidFiler + "'");

	if (!select())
	{
		qDebug() << "error on select in MessageController::setFilterOnJid";
	}
}

QVariant MessageController::data ( const QModelIndex & requestedIndex, int role ) const
{
	if(requestedIndex.row() >= rowCount())
	{
		//qDebug() << "return empty string!";
		return QString("");
	}

	if(role < Qt::UserRole)
	{
		QVariant data = QSqlTableModel::data(requestedIndex, role);
		//qDebug() << "return role string" << data;
		return data;
	}

	QModelIndex modelIndex = this->index(requestedIndex.row(), role - Qt::UserRole - 1 );

#if 0
	if (isDirty(modelIndex))
		qDebug() << "index is dirty!" << modelIndex;
#endif

	QVariant editData = QSqlQueryModel::data(modelIndex, Qt::EditRole);

	//qDebug() << "return edit data " << editData << " from index " << requestedIndex << " rowCount: " << rowCount();

	return editData;
}

// Role names are set to whatever your db table's column names are.
//
void MessageController::generateRoleNames()
{
	roles_.clear();
	for (int i = 0; i < columnCount(); i++)
	{
		roles_[Qt::UserRole + i + 1] = QVariant(headerData(i, Qt::Horizontal).toString()).toByteArray();
	}
}

QHash<int, QByteArray> MessageController::roleNames() const
{
	return roles_;
}

void MessageController::setTable ( const QString &table_name )
{
	QSqlTableModel::setTable(table_name);
	generateRoleNames();
}

void MessageController::addMessage(const QString &id, const QString &jid, const QString &message, unsigned int direction)
{
	QSqlRecord record = this->record();

    record.setValue("id", id);
	record.setValue("jid", jid);
	record.setValue("message", message);
	record.setValue("direction", direction);
    record.setValue("timestamp", QDateTime::currentDateTime().toTime_t());

	if (! this->insertRecord(-1, record))
	{
		qDebug() << "error on insert!";
	}
	else
	{
		if (! this->submitAll())
		{
			qDebug() << this->lastError().databaseText();
			qDebug() << this->lastError().driverText();
			qDebug() << this->lastError().text();
		}
		else
		{
			//qDebug() << "Success on adding message";
		}
	}

	// update the model with the changes of the database
	if (select())
	{
		if (direction == 1)
		{
			emit signalMessageReceived(0, jid, message);
		}
	}
	else
	{
		qDebug() << "error on select in MessageController::addMessage";
	}

//	database_->dumpDataToStdOut();
}

void MessageController::markMessageReceived(QString const &id)
{
    int row = getRowNumberForId(id);

    if (row >= 0) // only on found messages
    {
        QSqlRecord record = this->record(row);
        record.setValue("isreceived", true);

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

int MessageController::getRowNumberForId(QString const &id)
{
    int returnValue = -1;

    for (int row = 0; row < rowCount(); row++)
    {
        QString idInModel = record(row).value("id").toString();

        if (idInModel == id)
        {
            returnValue = row;
        }
    }

    return returnValue;
}

void MessageController::printSqlError()
{
    qDebug() << this->lastError().databaseText();
    qDebug() << this->lastError().driverText();
    qDebug() << this->lastError().text();
}
