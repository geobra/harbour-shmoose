#ifndef HTTPFILEUPLOADER_H
#define HTTPFILEUPLOADER_H

#include <QObject>
#include <QNetworkReply>

#include "../../Settings.h"

class QFile;
class QNetworkAccessManager;
class QNetworkRequest;

class HttpFileUploader : public QObject
{
    Q_OBJECT
public:
    HttpFileUploader(Settings * settings, QObject *parent = 0);
    ~HttpFileUploader();

    void upload(QString url, QFile *file);

signals:
    void updateStatus(QString status);
    void uploadSuccess();
    void errorOccurred();

public slots:
    void putFinished(QNetworkReply* reply);

    void displayProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();

private:
    Settings * settings_;
    QNetworkAccessManager* networkManager_;
    QNetworkRequest* request_;

    QFile* file_;
};

#endif // HTTPFILEUPLOADER_H
