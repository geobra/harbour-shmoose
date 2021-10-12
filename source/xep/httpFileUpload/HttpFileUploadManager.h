#ifndef HTTPFILEUPLOADMANAGER_H
#define HTTPFILEUPLOADMANAGER_H

#include <Swiften/Swiften.h>

#include <QObject>

class QFile;
class FileWithCypher;
class HttpFileUploader;

class HttpFileUploadManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpFileUploadManager(QObject *parent = 0);

    bool requestToUploadFileForJid(QString const &file, const QString &jid, bool encryptFile);
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
    void fileUploadFailedForJidToUrl();
    void showStatus(QString, QString);

public slots:
    void updateStatusString(QString string);
    void successReceived(QString string);
    void errorReceived();
    void setCompressImages(bool CompressImages);
    void setLimitCompression(unsigned int limitCompression);

private slots:
    void generateStatus(QString status);

private:
    HttpFileUploader* httpUpload_;
    void requestHttpUploadSlot();
    void handleHttpUploadResponse(const std::string response);

    bool createAttachmentPath();
    QString createTargetFileName(QString source, QString suffix="");

    static bool encryptFile(QFile &file, QByteArray &ivAndKey);

    bool serverHasFeatureHttpUpload_;
    unsigned int maxFileSize_;

    FileWithCypher* file_;
    QString jid_;

    Swift::Client* client_;
    Swift::JID uploadServerJid_;

    QString statusString_;
    QString getUrl_;

    bool busy_;
    bool encryptFile_;

    QString fileType_;

    bool compressImages_;
    unsigned int limitCompression_;
};

#endif // HTTPFILEUPLOADMANAGER_H
