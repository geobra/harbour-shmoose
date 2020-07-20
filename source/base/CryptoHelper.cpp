#include "CryptoHelper.h"

#include <QCryptographicHash>
#include <QRegularExpression>

const QString CryptoHelper::getHashOfString(const QString& str, bool appendExtention)
{
    QString hash = QString(QCryptographicHash::hash((str.toUtf8()),QCryptographicHash::Md5).toHex());

    if (appendExtention == true)
    {
        auto index = str.lastIndexOf('.');
        if (index < str.length() - 1 && index > str.length() - 7) // found . within the last 6 chars (. and 5 extension chars)
        {
            QStringRef subString(&str, index, str.length() - index);
            hash += subString;
        }
    }

    return hash;
}
