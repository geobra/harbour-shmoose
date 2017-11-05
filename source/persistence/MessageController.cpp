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
    setTable(Database::sqlMsgName_);

    // FIXME get column number from name!
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
    setFilter(Database::sqlJid_ + " = '" + jidFiler + "'");

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

// FIXME use direction enum
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

        record.setValue(Database::sqlId_, id);
        record.setValue(Database::sqlJid_, jid);
        record.setValue(Database::sqlResource_, resource);
        record.setValue(Database::sqlMsgMessage_, message);
        record.setValue(Database::sqlMsgDirection_, direction);
        record.setValue(Database::sqlMsgType_, type);
        record.setValue(Database::sqlTimestamp_, QDateTime::currentDateTime().toTime_t());

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

QString MessageController::getJidOfMessageId(QString const &id)
{
    QString jid = "";

    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("SELECT " + Database::sqlJid_ + " FROM " + Database::sqlMsgName_ + " WHERE " + Database::sqlId_ + " = \"" + id + "\""))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        while (query.next())
        {
            jid = query.value(0).toString();
            break;
        }
    }

    //qDebug() << "found " << jid << "for msg id " << id;

    return jid;
}

int MessageController::getStateOfMessageId(QString const &id)
{
    int msgState = MESSAGE_STATE_DEFAULT;

    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("SELECT " + Database::sqlMsgState_ + " FROM " + Database::sqlMsgName_ + " WHERE " + Database::sqlId_ + " = \"" + id + "\""))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        while (query.next())
        {
            msgState = query.value(0).toInt();
            break;
        }
    }

    qDebug() << "found state: " << msgState << "for msg id " << id;

    return msgState;
}

void MessageController::remarkMessageToReceivedForJidOfId(QString const &id)
{
    QString jid = getJidOfMessageId(id);

    if ( jid.contains("@") ) // jid was found
    {
        QSqlQuery query(*(database_->getPointer()));
        QString sqlQuery = "UPDATE " + Database::sqlMsgName_ + " SET " + Database::sqlMsgState_ + " = " + QString::number(MESSAGE_STATE_RECEIVED)
                + " WHERE " + Database::sqlMsgState_ + " = " + QString::number(MESSAGE_STATE_DISPLAYED) + " AND " + Database::sqlJid_ + " = \"" + jid + "\"";

        if (! query.exec(sqlQuery))
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
}

void MessageController::markMessageDisplayedConfirmed(QString const &id)
{
    setMessageStateOfId(id, MESSAGE_STATE_DISPLAYED_CONFIRMED);
}

void MessageController::markMessageDisplayed(QString const &id)
{
    // first, remark previous last displayed msg to just received for the jid of that msgId
    remarkMessageToReceivedForJidOfId(id);

    setMessageStateOfId(id, MESSAGE_STATE_DISPLAYED);
}

void MessageController::markMessageReceived(QString const &id)
{
    setMessageStateOfId(id, MESSAGE_STATE_RECEIVED);
}

void MessageController::markMessageSent(QString const &id)
{
    setMessageStateOfId(id, MESSAGE_STATE_SENT);
}

/*
 * valid state changes:
 * DEFAULT (0) -> DISPLAYED_CONFIRMED (-1)
 * or new state is bigger then current one
 */
void MessageController::setMessageStateOfId(QString const &id, int const state)
{
    int currentState = getStateOfMessageId(id);

    if ( (currentState == MESSAGE_STATE_DEFAULT && state == MESSAGE_STATE_DISPLAYED_CONFIRMED) ||
         (state > currentState)
         )
    {
        QSqlQuery query(*(database_->getPointer()));
        if (! query.exec("UPDATE " + Database::sqlMsgName_ + " SET \"" + Database::sqlMsgState_ + "\" = " + QString::number(state) + " WHERE id = \"" + id +"\""))
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
}

bool MessageController::isMessageIdInDatabase(QString const &id)
{
    bool returnValue = false;

    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("select " + Database::sqlId_ + " from " + Database::sqlMsgName_ + " WHERE " + Database::sqlId_ + " = \"" + id +"\""))
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

QPair<QString, int> MessageController::getNewestReceivedMessageIdAndStateOfJid(QString const &jid)
{
    QString msgId = "";
    int msgState = 0;

    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("SELECT " + Database::sqlId_ + ", " + Database::sqlMsgState_ + " FROM " + Database::sqlMsgName_
                     + " WHERE " + Database::sqlJid_ + " = \"" + jid + "\" AND " + Database::sqlMsgDirection_ + " = " + QString::number(MESSAGE_DIRECTION_INCOMING)
                     + " ORDER BY " + Database::sqlTimestamp_ + " DESC LIMIT 1"))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        while (query.next())
        {
            msgId = query.value(0).toString();
            msgState = query.value(1).toInt();
            break;
        }
    }

    return qMakePair<QString, int>(msgId, msgState);
}

void MessageController::printSqlError()
{
    qDebug() << this->lastError().databaseText();
    qDebug() << this->lastError().driverText();
    qDebug() << this->lastError().text();
}
