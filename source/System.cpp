#include "System.h"

#include <QStandardPaths>
#include <QDir>

QString System::getAttachmentPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "attachments";
}

QString System::getAvatarPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "avatar";
}
