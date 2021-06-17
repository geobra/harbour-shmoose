#ifndef CRYPTOHELPER_H
#define CRYPTOHELPER_H

#include <QString>

class CryptoHelper
{
public:
    static const QString getHashOfString(const QString& str, bool appendExtention = false);
    static bool aesDecrypt(const QString &ivAndKey, const QByteArray &in, QByteArray &out);
};

#endif // CRYPTOHELPER_H
