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
        ret_val = gcry_cipher_open(&cipherHd_, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_GCM, 0);

        if(ret_val) {
            qDebug() << "failed to initialize gcrypt";
            goto cleanup;
        }

        gcry_create_nonce(iv.data(), iv.size());
        gcry_create_nonce(key.data(), key.size());

        ivAndKey_ = iv.toHex() + key.toHex();

        ret_val = gcry_cipher_setkey(cipherHd_, key.constData(), key.size());
        if (ret_val) {
            qDebug() << "failed to set key ";
            goto cleanup;
        }

        ret_val = gcry_cipher_setiv(cipherHd_, iv.constData(), iv.size());
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

    // Supports 2 modes (IV on 12 or 16 bytes) to be compatible with some versions of Conversations
    if(ivAndKey.count() == 88 || ivAndKey.count() == 96)
    {
        QByteArray iv;
        QByteArray key;
        ivAndKey_ = ivAndKey; 

        iv = QByteArray::fromHex(ivAndKey.left(ivAndKey.count()-64).toLatin1());
        key = QByteArray::fromHex(ivAndKey.right(64).toLatin1());

        ret_val = gcry_cipher_open(&cipherHd_, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_GCM, 0);

        if(ret_val) {
            qDebug() << "failed to initialize gcrypt";
            goto cleanup;
        }

        ret_val = gcry_cipher_setkey(cipherHd_, key.constData(), key.size());
        if (ret_val) {
            qDebug() << "failed to set key";
            goto cleanup;
        }

        ret_val = gcry_cipher_setiv(cipherHd_, iv.constData(), iv.size());
        if (ret_val) {
            qDebug() << "failed to set iv";
            goto cleanup;
        } 
    }
    else
    {
        qDebug() << "unsupported encryption. iv+key length:" << ivAndKey.count();
        return false;
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


    if(cipherHd_ && !isWritable())
    {
        gcry_error_t ret_val = 0;


        if(readBytes > 0)
        {
            ret_val = gcry_cipher_encrypt(cipherHd_, data, readBytes, nullptr, 0);

            if (ret_val) {
                qDebug() << "failed to encrypt";
            }
        }
        else if((pos() + 16 ) == size())
        {
            readBytes = 16;

            ret_val = gcry_cipher_gettag(cipherHd_, data, readBytes);

            if(ret_val) {
                qDebug() << "failed to get authentication tag"; 
            }
        }
    }

    return readBytes;
}


qint64 FileWithCypher::writeData(const char *data, qint64 len) 
{
    QByteArray out(data, len);

    if(cipherHd_)
    {
        qint64 bytesToDecrypt = len;

        //don't decrypt the authentication tag
        if(pos()+len > expectedSize_-16)
        {
            bytesToDecrypt = (expectedSize_-16) - pos();
        }

        if(bytesToDecrypt > 0)
        {
            if( gcry_cipher_decrypt(cipherHd_, out.data(), bytesToDecrypt, nullptr, 0) ) {
                qDebug() << "failed to decrypt";
            }
        }
    }

    return QFile::writeData(out.constData(), out.size());
}


void FileWithCypher::setExpectedSize(qint64 expectedSize)
{
    expectedSize_ = expectedSize;
}


qint64  FileWithCypher::size() const
{
    qint64 fileSize = QFile::size();

    if(cipherHd_ && !isWritable())
    {
        // When reading a file to upload it, increase its size to send authentication tag
        // this is requested by xep-0454

       fileSize += 16;
    }

   return fileSize;
}


void  FileWithCypher::close()  
{
    // Before closing a file, if this is a download, remove authentication tag and check it
    if(cipherHd_ && isWritable())
    {
        if(seek(size() - 16))
        {
            gcry_error_t ret_val = 0;

            QByteArray authenticationTag = read(16);

            ret_val = gcry_cipher_checktag (cipherHd_, authenticationTag.constData(), authenticationTag.size());
            if(ret_val)
            {
                qDebug() << "invalid authentication Tag" << endl;
                // data are kept in the file. May be the authentication tag is missing in the source 
            }
            else
            {
                // Valid Authentication tag => remove it
                if(! resize(size() - 16)) {
                    qDebug() << "resize failed" << endl;
                }
            }
        }
    }

    QFile::close();
}
