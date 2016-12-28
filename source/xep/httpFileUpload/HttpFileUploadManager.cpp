#include "HttpFileUploadManager.h"
#include "XmlHttpUploadContentHandler.h"
#include "HttpFileuploader.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlSimpleReader>
#include <QDebug>

HttpFileUploadManager::HttpFileUploadManager(QObject *parent) : QObject(parent),
	httpUpload_(new HttpFileUploader(this)),
	severHasFeatureHttpUpload_(false), maxFileSize_(0),
	file_(new QFile(this)), jid_(""), client_(NULL),
	statusString_(""), getUrl_(""), busy_(false)
{
	connect(httpUpload_, SIGNAL(updateStatus(QString)), this, SLOT(updateStatusString(QString)));
	connect(httpUpload_, SIGNAL(uploadSuccess()), this, SLOT(successReceived()));
	connect(httpUpload_, SIGNAL(errorOccurred()), this, SLOT(errorReceived()));
}

void HttpFileUploadManager::setClient(Swift::Client* client)
{
	client_ = client;
}

bool HttpFileUploadManager::generateDownloadUrlToFileForJid(const QString &file, const QString &jid)
{
	bool returnValue = false;

	if (busy_ == false && client_ != NULL && severHasFeatureHttpUpload_ == true)
	{
		busy_ = true;
		returnValue = true;

		file_->setFileName(file);
		jid_ = jid;

		requestHttpUploadSlot();
	}

	return returnValue;
}

void HttpFileUploadManager::updateStatusString(QString string)
{
	statusString_ = string;
}

QString HttpFileUploadManager::getStatus()
{
	return statusString_;
}

void HttpFileUploadManager::setSeverHasFeatureHttpUpload(bool hasFeature)
{
	severHasFeatureHttpUpload_ = hasFeature;
}

bool HttpFileUploadManager::getSeverHasFeatureHttpUpload()
{
	return severHasFeatureHttpUpload_;
}

void HttpFileUploadManager::setMaxFileSize(unsigned int maxFileSize)
{
	maxFileSize_ = maxFileSize;
}

unsigned int HttpFileUploadManager::getMaxFileSize()
{
	return maxFileSize_;
}

void HttpFileUploadManager::requestHttpUploadSlot()
{
	QString basename = QFileInfo(file_->fileName()).baseName() + "." + QFileInfo(file_->fileName()).completeSuffix();
	std::string uploadRequest = "<request xmlns='urn:xmpp:http:upload'>"
		 + std::string("<filename>") + basename.toStdString() + std::string("</filename>")
		 + std::string("<size>") + std::to_string(file_->size()) + std::string("</size></request>");

	Swift::RawRequest::ref httpUploadRequest = Swift::RawRequest::create(Swift::IQ::Type::Get,
																		 Swift::JID(client_->getJID().getDomain()),
																		 uploadRequest,
																		 client_->getIQRouter());
	httpUploadRequest->onResponse.connect(boost::bind(&HttpFileUploadManager::handleHttpUploadResponse, this, _1));
	httpUploadRequest->send();
}

void HttpFileUploadManager::handleHttpUploadResponse(const std::string response)
{
	//qDebug() << "HttpFileUploadManager::handleHttpUploadResponse: " << QString::fromStdString(response);

	QXmlSimpleReader* parser = new QXmlSimpleReader();
	XmlHttpUploadContentHandler* handler = new XmlHttpUploadContentHandler();

	parser->setContentHandler(handler);
	parser->setContentHandler(handler);

	QXmlInputSource xmlSource;
	xmlSource.setData(QString::fromStdString(response));

	if(parser->parse(xmlSource))
	{
		qDebug() << "get: " << handler->getGetUrl();
		qDebug() << "put: " << handler->getPutUrl();

		if (QUrl(handler->getPutUrl()).isValid() == true && QUrl(handler->getGetUrl()).isValid() == true)
		{
			getUrl_ = handler->getGetUrl();

			// FIXME check file size
			httpUpload_->upload(handler->getPutUrl(), file_);
		}
	}
	else
	{
		qDebug() << "xml response parsing failed...";
	}
}

void HttpFileUploadManager::successReceived()
{
	busy_ = false;
	emit fileUploadedForJidToUrl(jid_, getUrl_);
}

void HttpFileUploadManager::errorReceived()
{
	busy_ = false;
}
