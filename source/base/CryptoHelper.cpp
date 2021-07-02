#include "CryptoHelper.h"

#include <QCryptographicHash>
#include <QRegularExpression>
#include <QUrl>

const QString CryptoHelper::getHashOfString(const QString& str, bool appendExtention)
{
    QString hash = QString(QCryptographicHash::hash((str.toUtf8()),QCryptographicHash::Md5).toHex());

    if (appendExtention == true)
    {
        QString fileName(QUrl(str).fileName());

        auto index = fileName.lastIndexOf('.');

        if (index >=0) // found . 
        {
            hash += fileName.mid(index);
        }
    }

    return hash;
}
