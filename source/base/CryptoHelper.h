#ifndef CRYPTOHELPER_H
#define CRYPTOHELPER_H

#include <QString>

class CryptoHelper
{
public:
    static const QString getHashOfString(const QString& str, bool appendExtention = false);
};

#endif // CRYPTOHELPER_H
