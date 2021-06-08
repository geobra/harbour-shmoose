#pragma once

#include <QCameraImageCapture>

class QCamera;

class CapturePicture : public QObject
{
    Q_OBJECT

public:
    CapturePicture(const QString path, QObject *parent);

public slots:
    void shot(bool ready);
    void onError(int id, QCameraImageCapture::Error error, const QString &errorString);

signals:
    void readyForCaptureChanged(bool);
    void imageSaved(int id, const QString &fileName);

private:
    const QString targetPath_;
    QCamera* camera_{nullptr};
    QCameraImageCapture* imageCapture_{nullptr};
};
