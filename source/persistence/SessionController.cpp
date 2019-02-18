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
}

bool SessionController::setup()
{
    bool returnValue = true;

    setEditStrategy(QSqlTableModel::OnRowChange);
    setTable("sessions");
    setSort(2, Qt::DescendingOrder);
    if (!select())
    {
        qDebug() << "error on select in SessionController::setup";
        returnValue = false;
    }

    return returnValue;
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

unsigned int SessionController::getNumberOfUnreadMessagesForJid(QString const &jid)
{
    unsigned int returnValue = 0;

    QSqlQuery query(*(database_->getPointer()));
    if (! query.exec("SELECT unreadmessages from sessions WHERE jid = \"" + jid +"\""))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        int idMsg = query.record().indexOf("unreadmessages");
        while (query.next())
        {
            returnValue = query.value(idMsg).toInt();
        }
    }
    return returnValue;
}

void SessionController::updateNumberOfUnreadMessages(QString const &jid, unsigned int unreadMessages)
{
    QSqlQuery query(*(database_->getPointer()));

    if (! query.exec("UPDATE sessions SET \"unreadmessages\" = " + QString::number(unreadMessages) + " WHERE jid = \"" + jid +"\""))
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
            qDebug() << "error on select in SessionController::updateNumberOfUnreadMessages";
        }
    }
}

void SessionController::updateSession(QString const &jid, QString const &lastMessage)
{
    unsigned int unreadMessages = getNumberOfUnreadMessagesForJid(jid);

    if (jid != currentChatPartner_)
    {
        unreadMessages++;
    }

    QSqlQuery query(*(database_->getPointer()));
    QString sqlCommand =
            "REPLACE into sessions (jid, lastmessage, timestamp, unreadmessages) VALUES ( \""
            + jid + "\", \"" + lastMessage.simplified() + "\", "
            + QString::number(QDateTime::currentDateTime().toTime_t()) + ", "
            + QString::number(unreadMessages) + " )";

    //qDebug() << sqlCommand;

    if (! query.exec(sqlCommand))
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
            qDebug() << "error on select in SessionController::updateSession";
        }
    }
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
