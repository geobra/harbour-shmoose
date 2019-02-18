#include "ImageProcessing.h"

#include <QImage>
#include <QImageReader>
#include <QBuffer>

#include <QDebug>

ImageProcessing::ImageProcessing()
{

}

// maxSize = 0 means dont care
bool ImageProcessing::prepareImageForSending(QString source, QString target, unsigned int maxSize)
{
    bool returnValue = false;
    unsigned int loopCount = 0;

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (buffer.open(QIODevice::WriteOnly) == true)
    {
        QImageReader imgreader(source);
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        imgreader.setAutoTransform(true);
#endif
        QImage img = imgreader.read();
        if (img.save(&buffer, "JPG") == true)
        {
            qDebug() << "buffer size: " << buffer.size();

            while (maxSize != 0 && buffer.size() > maxSize && loopCount < 15 /* just in case of an error to break the loop */)
            {
                // scale image down by half size
                bytes.clear();
                buffer.reset();

                img = img.scaled(img.width() / 2, img.height() / 2, Qt::KeepAspectRatio);
                img.save(&buffer, "JPG");

                qDebug() << "loop " << loopCount;

                loopCount++;
            }
            buffer.close();

            if (loopCount < 15 && img.save(target, "JPG"))
            {
                qDebug() << "newimg: " << img.size() << ", bytes: " << buffer.size();
                returnValue = true;
            }
        }
    }

    return returnValue;
}

QStringList ImageProcessing::getKnownImageTypes()
{
    QStringList list;
    list << "jpg" << "JPG" << "jpeg" << "JPEG" << "png" << "PNG" << "gif" << "GIF";

    return list;
}
