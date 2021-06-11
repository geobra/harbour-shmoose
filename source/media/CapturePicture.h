#pragma once

#include <QCameraImageCapture>

class QCamera;

class CapturePicture : public QObject
{
    Q_OBJECT

public:
    CapturePicture(const QString path, QObject *parent);
    void doShot();

private slots:
    void shot(bool ready);
    void onError(int id, QCameraImageCapture::Error error, const QString &errorString);
    void shutdownCamera();

private:
    const QString targetPath_;
    QCamera* camera_{nullptr};
    QCameraImageCapture* imageCapture_{nullptr};
};
