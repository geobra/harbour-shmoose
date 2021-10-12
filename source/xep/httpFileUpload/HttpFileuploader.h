#ifndef HTTPFILEUPLOADER_H
#define HTTPFILEUPLOADER_H

#include "FileWithCypher.h"
#include <QObject>
#include <QNetworkReply>

class QNetworkAccessManager;
class QNetworkRequest;
class QFile;

class HttpFileUploader : public QObject
{
    Q_OBJECT
public:
    HttpFileUploader(QObject *parent = 0);
    ~HttpFileUploader();

    void upload(QString url, FileWithCypher *file);

signals:
    void updateStatus(QString status);
    void uploadSuccess(QString ivAndKey);
    void errorOccurred();

public slots:
    void putFinished(QNetworkReply* reply);

    void displayProgress(qint64 bytesReceived, qint64 bytesTotal);
    void error(QNetworkReply::NetworkError code);
    void finished();

private:
    QNetworkAccessManager* networkManager_;
    QNetworkRequest* request_;

    FileWithCypher* file_;

};

#endif // HTTPFILEUPLOADER_H
