#ifndef HTTPFILEUPLOADMANAGER_H
#define HTTPFILEUPLOADMANAGER_H

#include <Swiften/Swiften.h>

#include <QObject>

class QFile;
class HttpFileUploader;

class HttpFileUploadManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpFileUploadManager(QObject *parent = 0);

    bool requestToUploadFileForJid(QString const &file, const QString &jid);
    QString getStatus();

    void setupWithClient(Swift::Client* client);

    void setServerHasFeatureHttpUpload(bool hasFeature);
    bool getServerHasFeatureHttpUpload();

    void setUploadServerJid(Swift::JID const & uploadServerJid);
    Swift::JID getUploadServerJid();

    void setMaxFileSize(unsigned int maxFileSize);
    unsigned int getMaxFileSize();

signals:
    void fileUploadedForJidToUrl(QString, QString, QString);

public slots:
    void updateStatusString(QString string);
    void successReceived();
    void errorReceived();

private:
    HttpFileUploader* httpUpload_;
    void requestHttpUploadSlot();
    void handleHttpUploadResponse(const std::string response);

    bool createAttachmentPath();
    QString createTargetImageName(QString source);

    bool serverHasFeatureHttpUpload_;
    unsigned int maxFileSize_;

    QFile* file_;
    QString jid_;

    Swift::Client* client_;
    Swift::JID uploadServerJid_;

    QString statusString_;
    QString getUrl_;

    bool busy_;
};

#endif // HTTPFILEUPLOADMANAGER_H
