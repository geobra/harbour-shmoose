#include "MessageController.h"
#include "Database.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>

#include <QDebug>

MessageController::MessageController(QObject *parent) : QSqlTableModel(parent)
{
}

MessageController::MessageController(Database *db, QObject *parent) :
	QSqlTableModel(parent, *(db->getPointer())), database_(db)
{
}

bool MessageController::setup()
{
    bool returnValue = true;

    setEditStrategy(QSqlTableModel::OnRowChange);
    setTable("messages");
    setSort(5, Qt::DescendingOrder); // 5 -> timestamp
    if (!select())
    {
        qDebug() << "error on select in MessageController::setup";
        returnValue = false;
    }

    return returnValue;
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

bool MessageController::addMessage(bool isGroupMessage, const QString &id, const QString &jid, const QString &resource, const QString &message, const QString &type, unsigned int direction)
{
    /* received group messages are special
     * - my own message into a room will be send back to me from the room
     * - on room (re)joun, the last n messages are (re)send to me
     * -> here we skip adding received messages if it was a group message AND the message id is already in the database
    */

    bool messageAdded = false;

    if ( (isGroupMessage == false)
         || ( (isGroupMessage == true) && (! isMessageIdInDatabase(id)) )
         )
    {
        messageAdded = true;

        QSqlRecord record = this->record();

        record.setValue("id", id);
        record.setValue("jid", jid);
        record.setValue("resource", resource);
        record.setValue("message", message);
        record.setValue("direction", direction);
        record.setValue("type", type);
        record.setValue("timestamp", QDateTime::currentDateTime().toTime_t());

        if (! this->insertRecord(-1, record))
        {
            messageAdded = false;
            printSqlError();
        }
        else
        {
            if (! this->submitAll())
            {
                messageAdded = false;
                printSqlError();
            }
        }

        // update the model with the changes of the database
        if (select())
        {
            if (direction == 1)
            {
                emit signalMessageReceived(id, jid, message);
            }
        }
        else
        {
            messageAdded = false;
            qDebug() << "error on select in MessageController::addMessage";
        }

        //database_->dumpDataToStdOut();
    }
    else
    {
        messageAdded = false;
    }

    return messageAdded;
}

void MessageController::markMessageReceived(QString const &id)
{
    markColumnOfId("isreceived", id);
}

void MessageController::markMessageSent(QString const &id)
{
    markColumnOfId("issent", id);
}

void MessageController::markColumnOfId(QString const &column, QString const &id)
{
    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("UPDATE messages SET \"" + column + "\" = 1 WHERE id = \"" + id +"\""))
    {
         qDebug() << query.lastError().databaseText();
         qDebug() << query.lastError().driverText();
         qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (select() != true)
        {
            qDebug() << "error on select in MessageController::addMessage";
        }
    }
}

bool MessageController::isMessageIdInDatabase(QString const &id)
{
    bool returnValue = false;

    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("select id from messages WHERE id = \"" + id +"\""))
    {
         qDebug() << query.lastError().databaseText();
         qDebug() << query.lastError().driverText();
         qDebug() << query.lastError().text();
    }
    else
    {
        while (query.next())
        {
            returnValue = true;
            break;
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
