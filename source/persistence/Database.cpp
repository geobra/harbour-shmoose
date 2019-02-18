#include "Database.h"

#include <QCoreApplication>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QDir>

#include <QDebug>

// messages table
const QString Database::sqlMsgName_ = "messages";                   // sql table name
const QString Database::sqlMsgMessage_ = "message";                 // plain msg
const QString Database::sqlMsgDirection_ = "direction";             // (1)ncomming, (0)utgoing
const QString Database::sqlMsgType_ = "type";                       // group / normal
const QString Database::sqlMsgState_ = "msgstate";                  // (-1) displayedConfirmed, (0) unknown, (1) send, (2) received, (3) displayed

// session table
const QString Database::sqlSessionName_ = "sessions";               // sql table name
const QString Database::sqlSessionLastMsg_ = "lastmessage";         // the content of the last message
const QString Database::sqlSessionUnreadMsg_ = "unreadmessages";    // number of unread messages

// common sql column names
const QString Database::sqlId_ = "id";                              // the msg id
const QString Database::sqlJid_ = "jid";                            // the jid
const QString Database::sqlResource_ = "resource";                  // resource of jid
const QString Database::sqlTimestamp_ = "timestamp";                // the unix timestamp



Database::Database(QObject *parent) : QObject(parent),
    databaseValid_(false), database_(QSqlDatabase::addDatabase("QSQLITE"))
{
}

bool Database::open(QString const &jid)
{
    QString databaseName = jid;
    databaseName.replace("@", "-at-");
    databaseName.append("." + qApp->applicationVersion());
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

            if (! database_.tables().contains( sqlMsgName_ ))
            {
                QString sqlCreateCommand = "create table " + sqlMsgName_ + " (" + sqlId_ + " TEXT, " + sqlJid_ + " TEXT, "
                        + sqlResource_ + " TEXT, " + sqlMsgMessage_ + " TEXT, " + sqlMsgDirection_ + " INTEGER, "
                        + sqlTimestamp_ + " INTEGER, " + sqlMsgType_ + " STRING, " + sqlMsgState_ + " INTEGER)";
                if (query.exec(sqlCreateCommand) == false)
                {
                    qDebug() << "Error creating message table";
                    databaseValid_ = false;
                }
            }

            if (! database_.tables().contains( sqlSessionName_ ))
            {
                // another table for the sessions
                QString sqlCreateCommand = "create table " + sqlSessionName_ + " (" + sqlJid_ + " TEXT PRIMARY KEY, " + sqlSessionLastMsg_ + " TEXT, "
                        + sqlTimestamp_ + " INTEGER, " + sqlSessionUnreadMsg_ + " INTEGER)";
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

    const unsigned int idCol = rec.indexOf(Database::sqlId_);
    const unsigned int jidCol = rec.indexOf(Database::sqlJid_);
    const unsigned int resourceCol = rec.indexOf(Database::sqlResource_);
    const unsigned int messageCol = rec.indexOf(Database::sqlMsgMessage_);
    const unsigned int directionCol = rec.indexOf(Database::sqlMsgDirection_);
    const unsigned int timeStampCol = rec.indexOf(Database::sqlTimestamp_);
    const unsigned int messageState = rec.indexOf(Database::sqlMsgState_);
    const unsigned int typeCol = rec.indexOf(Database::sqlMsgType_);


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
                 << query.value(messageState).toBool() << "\t";
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
