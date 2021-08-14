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
    void fileUploadedForJidToUrl(QString, QString, QString, QString);
    void startFileUploadForJidToUrl(QString, QString, QString, QString);
    void showStatus(QString, QString);

public slots:
    void updateStatusString(QString string);
    void successReceived(QString string);
    void errorReceived();

private slots:
    void generateStatus(QString status);

private:
    HttpFileUploader* httpUpload_;
    void requestHttpUploadSlot();
    void handleHttpUploadResponse(const std::string response);

    bool createAttachmentPath();
    QString createTargetFileName(QString source);

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
    QString msgId_;
};

#endif // HTTPFILEUPLOADMANAGER_H
