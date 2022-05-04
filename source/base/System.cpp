#include "System.h"
#include "Settings.h"

#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

QString System::getAttachmentPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "attachments";
}

QString System::getAvatarPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "avatar";
}

QString System::getOmemoPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "omemo";
}

QString System::getUniqueResourceId()
{
    QString resourceId = Settings().getResourceId();
 
    if(resourceId.isEmpty())
    {
        const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        const int randomStringLength = 4; 

        qsrand( QDateTime::currentDateTime().toTime_t() );

        QString randomString;
        for(int i=0; i<randomStringLength; ++i)
        {
           int index = qrand() % possibleCharacters.length();
           QChar nextChar = possibleCharacters.at(index);
           randomString.append(nextChar);
        }

        resourceId = randomString;
        Settings().setResourceId(resourceId);
    }

   return resourceId;
}