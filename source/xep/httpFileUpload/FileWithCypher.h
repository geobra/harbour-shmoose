#ifndef FILEWITHCYPHER_H
#define FILEWITHCYPHER_H

#include <QObject>
#include <QFile>

#include <gcrypt.h>

class FileWithCypher : public QFile
{
    Q_OBJECT

public:

    FileWithCypher(QObject *parent);
    FileWithCypher(const QString &name, QObject *parent = 0);
    ~FileWithCypher();

    bool initEncryptionOnRead(bool encrypt);
    bool initDecryptionOnWrite(const QString &ivAndKey);
    const QString &getIvAndKey();

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
 
    QString ivAndKey_;
    gcry_cipher_hd_t cipherHd_;
};

#endif // FILEWITHCYPHER_H
