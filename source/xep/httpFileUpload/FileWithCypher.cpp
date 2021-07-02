#include "FileWithCypher.h"


#include <QDebug>
#include <QFile>


FileWithCypher::FileWithCypher(QObject *parent) 
  : QFile(parent)
{
    cipherHd_ = nullptr;
}


FileWithCypher::FileWithCypher(const QString &name, QObject *parent) 
  : QFile(name, parent)
{
    cipherHd_ = nullptr;
}

FileWithCypher::~FileWithCypher() 
{
   if(cipherHd_) 
    {
        gcry_cipher_close(cipherHd_);
        cipherHd_ = nullptr;
    }
}

bool FileWithCypher::initEncryptionOnRead(bool encrypt)
{
    QByteArray iv(12, '\0');
    QByteArray key(32, '\0');
    gcry_error_t ret_val = 0;

    
    if(cipherHd_) 
    {
        gcry_cipher_close(cipherHd_);
        cipherHd_ = nullptr;
        ivAndKey_ = "";
    }

    if(encrypt)
    {
    
        ret_val = gcry_cipher_open(&cipherHd_, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CTR, 0);

        if(ret_val) {
            qDebug() << "failed to initialize gcrypt";
            goto cleanup;
        }

        ret_val = gcry_cipher_open(&cipherHd_, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CTR, 0);

        if(ret_val) {
            qDebug() << "failed to initialize gcrypt";
            goto cleanup;
        }

        gcry_create_nonce(iv.data(), iv.size());
        gcry_create_nonce(key.data(), key.size());

        ivAndKey_ = iv.toHex() + key.toHex();
        iv += QByteArray::fromHex("00000002");


        ret_val = gcry_cipher_setkey(cipherHd_, key.constData(), key.size());
        if (ret_val) {
            qDebug() << "failed to set key ";
            goto cleanup;
        }

        ret_val = gcry_cipher_setctr(cipherHd_, iv.constData(), iv.size());
        if (ret_val) {
            qDebug() << "failed to set iv = " << iv.toHex();
            goto cleanup;
        } 
    }

    cleanup:
        return ret_val == 0;
}

bool FileWithCypher::initDecryptionOnWrite(const QString &ivAndKey)
{
    gcry_error_t ret_val = 0;
    
    if(cipherHd_) 
    {
        gcry_cipher_close(cipherHd_);
        cipherHd_ = nullptr;
    }

    if(ivAndKey.count() > 0)
    {
        QByteArray iv(12, '\0');
        QByteArray key(32, '\0');       

        if(ivAndKey.count() != 88) {
            qDebug() << "unsupported encryption";
            return false;
        }

        ivAndKey_ = ivAndKey; 

        iv = QByteArray::fromHex(ivAndKey.mid(0, 24).toLatin1()) + QByteArray::fromHex("00000002");
        key = QByteArray::fromHex(ivAndKey.mid(24, 64).toLatin1());

        ret_val = gcry_cipher_open(&cipherHd_, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CTR, 0);

        if(ret_val) {
            qDebug() << "failed to initialize gcrypt";
            goto cleanup;
        }

        ret_val = gcry_cipher_setkey(cipherHd_, key.constData(), key.size());
        if (ret_val) {
            qDebug() << "failed to set key";
            goto cleanup;
        }

        ret_val = gcry_cipher_setctr(cipherHd_, iv.constData(), iv.size());
        if (ret_val) {
            qDebug() << "failed to set ctr";
            goto cleanup;
        } 
    }
    
cleanup:
    return ret_val == 0;
}

const QString &FileWithCypher::getIvAndKey()
{
    return ivAndKey_;
}


qint64 FileWithCypher::readData(char *data, qint64 maxSize) 
{
    qint64 readBytes = QFile::readData(data, maxSize);


    if(cipherHd_)
    {
        gcry_error_t ret_val = 0;

        ret_val = gcry_cipher_encrypt(cipherHd_, data, readBytes, nullptr, 0);

        if (ret_val) {
            qDebug() << "failed to encrypt";
        }
    }

    return readBytes;
}


qint64 FileWithCypher::writeData(const char *data, qint64 len) 
{
    QByteArray out(data, len);

    if(cipherHd_)
    {
        if( gcry_cipher_decrypt(cipherHd_, out.data(), out.size(), nullptr, 0) ) {
            qDebug() << "failed to decrypt";
        }
    }

    return QFile::writeData(out.constData(), out.size());
}

