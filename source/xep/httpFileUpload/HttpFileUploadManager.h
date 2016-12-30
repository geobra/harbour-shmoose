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

	void setClient(Swift::Client* client);

	void setSeverHasFeatureHttpUpload(bool hasFeature);
	bool getSeverHasFeatureHttpUpload();

	void setMaxFileSize(unsigned int maxFileSize);
	unsigned int getMaxFileSize();

signals:
	void fileUploadedForJidToUrl(QString, QString);

public slots:
	void updateStatusString(QString string);
	void successReceived();
	void errorReceived();

private:
	HttpFileUploader* httpUpload_;
	void requestHttpUploadSlot();
	void handleHttpUploadResponse(const std::string response);

    QString getAttachmentPath();
    bool createAttachmentPath();
    QString createTargetImageName(QString source);

	bool severHasFeatureHttpUpload_;
	unsigned int maxFileSize_;

	QFile* file_;
	QString jid_;

	Swift::Client* client_;

	QString statusString_;
	QString getUrl_;

	bool busy_;
};

#endif // HTTPFILEUPLOADMANAGER_H
