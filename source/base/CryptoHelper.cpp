#include "CryptoHelper.h"

#include <QCryptographicHash>
#include <QRegularExpression>
#include <QDebug>

#include <gcrypt.h>

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

bool CryptoHelper::aesDecrypt(const QString& ivAndKey, const QByteArray &in, QByteArray &out)
{
    const QByteArray iv = QByteArray::fromHex(ivAndKey.mid(0, 24).toLatin1()) + QByteArray::fromHex("00000002");
    const QByteArray key = QByteArray::fromHex(ivAndKey.mid(24, 64).toLatin1());

    gcry_cipher_hd_t cipher_hd = nullptr;
    gcry_error_t ret_val;

    ret_val = gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CTR, 0);

    if(ret_val) {
        qDebug() << "failed to initialize gcrypt";
        goto cleanup;
    }

    if(key.count() != 32)
    {
        qDebug() << "invalid key size: " << key.count();
        goto cleanup;
    }

    if(iv.count() != 16)
    {
        qDebug() << "invalid iv size: " << iv.count();
        goto cleanup;

    }

    ret_val = gcry_cipher_setkey(cipher_hd, key.constData(), static_cast<size_t>(key.size()));
    if (ret_val) {
        qDebug() << "failed to set key";
        goto cleanup;
    }

    ret_val = gcry_cipher_setctr(cipher_hd, iv.constData(), static_cast<size_t>(iv.size()));
    if (ret_val) {
        qDebug() << "failed to set ctr";
        goto cleanup;
    }

    out.resize(in.size());

    ret_val = gcry_cipher_decrypt(cipher_hd, out.data(), static_cast<size_t>(out.size()), in.constData(), static_cast<size_t>(in.size()));
    if (ret_val) {
        qDebug() << "failed to decrypt";
        goto cleanup;
    }

cleanup:

  if(cipher_hd)
  {
        gcry_cipher_close(cipher_hd);
  }

  return ret_val == 0;
}
