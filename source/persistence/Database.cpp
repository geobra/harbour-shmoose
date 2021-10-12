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
const QString Database::security_ = "security";                     // 0: non (plain), 1: omemo

// session table
const QString Database::sqlSessionName_ = "sessions";               // sql table name
const QString Database::sqlSessionLastMsg_ = "lastmessage";         // the content of the last message
const QString Database::sqlSessionUnreadMsg_ = "unreadmessages";    // number of unread messages

// groupchatmarker (gcm) table
const QString Database::sqlGcmName_ = "groupchatmarkers";           // sql table name
const QString Database::sqlGcmChatMemberName_ = "chatmembername";   // the name of the group chat member. Is the resource after the room jid
const QString Database::sqlGcmState_ = "msgstate";                  // The message state. (0) unknown, (1) received, (2) displayed

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
                        + sqlTimestamp_ + " INTEGER, " + sqlMsgType_ + " STRING, " + sqlMsgState_ + " INTEGER, "
                        + security_ + " INTEGER)";
                if (query.exec(sqlCreateCommand) == false)
                {
                    qDebug() << "Error creating message table";
                    databaseValid_ = false;
                }
            }

            // another table for the sessions
            if (! database_.tables().contains( sqlSessionName_ ))
            {
                QString sqlCreateCommand = "create table " + sqlSessionName_ + " (" + sqlJid_ + " TEXT PRIMARY KEY, " + sqlSessionLastMsg_ + " TEXT, "
                        + sqlTimestamp_ + " INTEGER, " + sqlSessionUnreadMsg_ + " INTEGER)";
                if (query.exec(sqlCreateCommand) == false)
                {
                    qDebug() << "Error creating sessions table";
                    databaseValid_ = false;
                }
            }

            // another table for the groupchat states
            if (! database_.tables().contains( sqlGcmName_ ))
            {
                QString sqlCreateCommand = "create table " + sqlGcmName_ +
                        " (" + sqlId_ + " TEXT, " + sqlGcmChatMemberName_ + " TEXT, " + sqlTimestamp_ + " INTEGER, " + sqlGcmState_ + " INTEGER, " +
                        "PRIMARY KEY ( " + sqlId_ + ", " + sqlGcmChatMemberName_ + ") )";
                if (query.exec(sqlCreateCommand) == false)
                {
                    qDebug() << "Error creating groupchatstate table";
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
    QSqlQuery query(database_);
    QSqlRecord rec;

    query.prepare("select * from " + sqlMsgName_);
    query.exec();
    rec = query.record();

    qDebug() << "id:\t\tjid:\tresource:\tmessage:\tdirection\ttimestamp,\ttype,\tsent,\treceived,\tsecurity:";
    qDebug() << "---------------------------------------------------------------------------------------";
    while (query.next())
    {
        qDebug() << query.value(Database::sqlId_).toString() << "\t"
                 << query.value(Database::sqlJid_).toString() << "\t"
                 << query.value(Database::sqlResource_).toString() << "\t"
                 << query.value(Database::sqlMsgMessage_).toString() << "\t"
                 << query.value(Database::sqlMsgDirection_).toString() << "\t"
                 << query.value(Database::sqlTimestamp_).toInt() << "\t"
                 << query.value(Database::sqlMsgType_).toString() << "\t"
                 << query.value(Database::sqlMsgState_).toInt() << "\t"
                 << query.value(Database::security_).toBool() << "\t";
    }

#if 0
    query.prepare("select * from " + sqlSessionName_);
    query.exec();
    rec = query.record();

    qDebug() << "jid:\tlastmessage:\ttimestamp\tunreadmessages:";
    qDebug() << "---------------------------------------------------------------------------------------";
    while (query.next())
    {
        qDebug()
                << query.value(Database::sqlJid_).toString() << "\t"
                << query.value(Database::sqlSessionLastMsg_).toString() << "\t"
                << query.value(Database::sqlTimestamp_).toInt() << "\t"
                << query.value(Database::sqlSessionUnreadMsg_).toInt() << "\t";
    }
#endif
}
