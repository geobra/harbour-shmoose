#include "Database.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QDir>

#include <QDebug>

Database::Database(QObject *parent) : QObject(parent),
    databaseValid_(false), database_(QSqlDatabase::addDatabase("QSQLITE"))
{
}

bool Database::open(QString const &jid)
{
    QString databaseName = jid;
    databaseName.replace("@", "-at-");
    databaseName.append(".sql");

    if (! database_.isValid())
    {
        qDebug() << "Database error!";
        databaseValid_ = false;
    }
    else
    {
        QString dbName = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + databaseName;
        database_.setDatabaseName(dbName);
        if (database_.open() == false)
        {
            qDebug() << "Error open database!";
            databaseValid_ = false;
        }
        else
        {
            /* shmoose uses two table
             * one for all the messages, one for all the sessions
             *
             * + no table joins
             * + no complex queries
             * + fast
             * + standard qt classes
             * - minor redundant data
             */

            databaseValid_ = true; // will be set to false again on error

            // table for all the messages
            QSqlQuery query;

            QString messagesTable = "messages";
            if (! database_.tables().contains( messagesTable ))
            {
                // direction: (1)ncomming / (0)utgoing
                QString sqlCreateCommand = "create table " + messagesTable + " (id TEXT, jid TEXT, resource TEXT, message TEXT, direction INTEGER, timestamp INTEGER, type STRING, issent BOOL, isreceived BOOL)";
                if (query.exec(sqlCreateCommand) == false)
                {
                    qDebug() << "Error creating message table";
                    databaseValid_ = false;
                }
            }

            QString sessionsTable = "sessions";
            if (! database_.tables().contains( sessionsTable ))
            {
                // another table for the sessions
                QString sqlCreateCommand = "create table " + sessionsTable + " (jid TEXT PRIMARY KEY, lastmessage TEXT, timestamp INTEGER, unreadmessages INTEGER)";
                if (query.exec(sqlCreateCommand) == false)
                {
                    qDebug() << "Error creating sessions table";
                    databaseValid_ = false;
                }
            }
        }
    }

    return databaseValid_;
}

QSqlDatabase* Database::getPointer()
{
	return &database_;
}

bool Database::isValid()
{
	return databaseValid_;
}

void Database::dumpDataToStdOut() const
{

	QSqlQuery query("select * from messages", database_);
	QSqlRecord rec = query.record();

	const unsigned int idCol = rec.indexOf("id");
	const unsigned int jidCol = rec.indexOf("jid");
    const unsigned int resourceCol = rec.indexOf("resource");
	const unsigned int messageCol = rec.indexOf("message");
	const unsigned int directionCol = rec.indexOf("direction");
	const unsigned int timeStampCol = rec.indexOf("timestamp");
	const unsigned int isSentCol = rec.indexOf("issent");
	const unsigned int isReceivedCol = rec.indexOf("isreceived");
	const unsigned int typeCol = rec.indexOf("type");


    qDebug() << "id:\t\tjid:\tresource:\tmessage:\tdirection\ttimestamp,\ttype,\tsent,\treceived:";
	qDebug() << "---------------------------------------------------------------------------------------";
	while (query.next())
	{
		qDebug() << query.value(idCol).toString() << "\t"
				 << query.value(jidCol).toString() << "\t"
                 << query.value(resourceCol).toString() << "\t"
				 << query.value(messageCol).toString() << "\t"
				 << query.value(directionCol).toString() << "\t"
				 << query.value(timeStampCol).toInt() << "\t"
				 << query.value(typeCol).toString() << "\t"
				 << query.value(isSentCol).toBool() << "\t"
				<< query.value(isReceivedCol).toBool() << "\t";
	}

#if 0
	QSqlQuery query("select * from sessions", database_);
	QSqlRecord rec = query.record();

	const unsigned int jidCol = rec.indexOf("jid");
	const unsigned int messageCol = rec.indexOf("lastmessage");
	const unsigned int tsCol = rec.indexOf("timestamp");
	const unsigned int unreadMsgCol = rec.indexOf("unreadmessages");

	qDebug() << "id:\tjid:\tlastmessage:\ttimestamp\tunreadmessages:";
	qDebug() << "---------------------------------------------------------------------------------------";
	while (query.next())
	{
		qDebug()
				 << query.value(jidCol).toString() << "\t"
				 << query.value(messageCol).toString() << "\t"
				 << query.value(tsCol).toInt() << "\t"
				 << query.value(unreadMsgCol).toInt() << "\t";
	}
#endif
}
