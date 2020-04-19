#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <QString>

class ImageProcessing
{
public:
    static bool prepareImageForSending(QString source, QString target, unsigned int maxSize);
    static QStringList getKnownImageTypes();

private:
    ImageProcessing();
};

#endif // IMAGEPROCESSING_H
