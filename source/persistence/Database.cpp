#include "Database.h"

#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

Database::Database(QObject *parent) : QObject(parent), databaseValid_(true)
{
	database_ = QSqlDatabase::addDatabase("QSQLITE");
	if (! database_.isValid())
	{
		qDebug() << "Database error!";
		databaseValid_ = false;
	}
	else
	{
		// TODO place in filesystem when tables get stable
		database_.setDatabaseName(":memory:");
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

			// table for all the messages
			QSqlQuery query;
			// direction: (1)ncomming / (0)utgoing
            QString sqlCreateCommand = "create table messages (id TEXT PRIMARY KEY, jid TEXT, message TEXT, direction INTEGER, timestamp INTEGER, isreceived BOOL)";
			if (query.exec(sqlCreateCommand) == false)
			{
				qDebug() << "Error creating message table";
				databaseValid_ = false;
			}

			// another table for the sessions
			sqlCreateCommand = "create table sessions (jid TEXT, lastmessage TEXT, timestamp INTEGER, unreadmessages INTEGER)";
			if (query.exec(sqlCreateCommand) == false)
			{
				qDebug() << "Error creating sessions table";
				databaseValid_ = false;
			}
		}
	}
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
	const unsigned int messageCol = rec.indexOf("message");
	const unsigned int directionCol = rec.indexOf("direction");
    const unsigned int timeStampCol = rec.indexOf("timestamp");
    const unsigned int isReceivedCol = rec.indexOf("isreceived");

    qDebug() << "id:\t\tjid:\tmessage:\t\tdirection\ttimestamp,\treceived:";
	qDebug() << "---------------------------------------------------------------------------------------";
	while (query.next())
	{
        qDebug() << query.value(idCol).toString() << "\t"
				 << query.value(jidCol).toString() << "\t"
				 << query.value(messageCol).toString() << "\t"
				 << query.value(directionCol).toString() << "\t"
                 << query.value(timeStampCol).toInt() << "\t"
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
