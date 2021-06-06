#include "CapturePicture.h"

#include <QCameraViewfinder>
#include <QCamera>
#include <QTimer>
#include <QDateTime>
#include <QDir>

#include <QDebug>

CapturePicture::CapturePicture(const QString path, QObject *parent) : QObject(parent), targetPath_(path)
{
    camera_ = new QCamera;

    imageCapture_ = new QCameraImageCapture(camera_);

    // wait for camera to get ready, print out capture errors
    connect(imageCapture_, SIGNAL(error(int, QCameraImageCapture::Error, const QString&)), this, SLOT(onError(int, QCameraImageCapture::Error, const QString)));
    connect(imageCapture_, SIGNAL(readyForCaptureChanged(bool)), this, SIGNAL(readyForCaptureChanged(bool)));

    camera_->setCaptureMode(QCamera::CaptureStillImage);
    camera_->start();
    camera_->searchAndLock();
    camera_->unlock();
}

void CapturePicture::shot(bool ready)
{
    if (ready == true && imageCapture_->isReadyForCapture() == true)
    {
        const QString file = targetPath_ + QDir::separator() + QString::number(QDateTime::currentMSecsSinceEpoch());;
        qDebug() << "shot! " << file;
        imageCapture_->capture(file);
    }
}

void CapturePicture::onError(int id, QCameraImageCapture::Error error, const QString &errorString)
{
    qDebug() << id << error << errorString;
}
