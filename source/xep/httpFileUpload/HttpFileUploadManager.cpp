#include "HttpFileUploadManager.h"
#include "XmlHttpUploadContentHandler.h"
#include "HttpFileuploader.h"
#include "ImageProcessing.h"
#include "System.h"
#include "CryptoHelper.h"
#include "FileWithCypher.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlSimpleReader>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>

#include <QDebug>




HttpFileUploadManager::HttpFileUploadManager(QObject *parent) : QObject(parent),
    httpUpload_(new HttpFileUploader(this)),
    serverHasFeatureHttpUpload_(false), maxFileSize_(0),
    file_(new FileWithCypher(this)), jid_(""), client_(nullptr),
    uploadServerJid_(""), statusString_(""), getUrl_(""), busy_(false)
{
    connect(httpUpload_, SIGNAL(updateStatus(QString)), this, SLOT(updateStatusString(QString)));
    connect(httpUpload_, SIGNAL(uploadSuccess(QString)), this, SLOT(successReceived(QString)));
    connect(httpUpload_, SIGNAL(errorOccurred()), this, SLOT(errorReceived()));

    connect(httpUpload_, SIGNAL(updateStatus(QString)), this, SLOT(generateStatus(QString)));

    busy_ = (! this->createAttachmentPath());
}

void HttpFileUploadManager::setupWithClient(Swift::Client* client)
{
    client_ = client;
}

bool HttpFileUploadManager::requestToUploadFileForJid(const QString &file, const QString &jid, bool encryptFile)
{
    bool returnValue = false;

    if (busy_ == false && client_ != nullptr && serverHasFeatureHttpUpload_ == true)
    {
        QFile inputFile(file);
        QString fileToUpload = createTargetImageName(file);

        // don't resize image if server can handle it
        if(inputFile.size() < getMaxFileSize()) 
            returnValue = inputFile.copy(fileToUpload);
        else 
            returnValue = ImageProcessing::prepareImageForSending(file, fileToUpload, getMaxFileSize());

        if(returnValue)
        {
            busy_ = true;
            encryptFile_ = encryptFile;
            returnValue = true;

            file_->setFileName(fileToUpload);
            jid_ = jid;

            requestHttpUploadSlot();
        }
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

void HttpFileUploadManager::setServerHasFeatureHttpUpload(bool hasFeature)
{
    serverHasFeatureHttpUpload_ = hasFeature;
}


bool HttpFileUploadManager::getServerHasFeatureHttpUpload()
{
    return serverHasFeatureHttpUpload_;
}

void HttpFileUploadManager::setUploadServerJid(Swift::JID const & uploadServerJid)
{
    uploadServerJid_ = uploadServerJid;
}

Swift::JID HttpFileUploadManager::getUploadServerJid()
{
    return uploadServerJid_;
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
    if (client_ != nullptr)
    {
        QString basename = QFileInfo(file_->fileName()).baseName() + "." + QFileInfo(file_->fileName()).completeSuffix();
        std::string uploadRequest = "<request xmlns='urn:xmpp:http:upload'>"
                + std::string("<filename>") + basename.toStdString() + std::string("</filename>")
                + std::string("<size>") + std::to_string(file_->size()) + std::string("</size></request>");

        Swift::RawRequest::ref httpUploadRequest = Swift::RawRequest::create(Swift::IQ::Type::Get,
                                                                             uploadServerJid_,
                                                                             uploadRequest,
                                                                             client_->getIQRouter());
        httpUploadRequest->onResponse.connect(boost::bind(&HttpFileUploadManager::handleHttpUploadResponse, this, _1));
        httpUploadRequest->send();
    }
}

void HttpFileUploadManager::handleHttpUploadResponse(const std::string response)
{
    //qDebug() << "HttpFileUploadManager::handleHttpUploadResponse: " << QString::fromStdString(response);

    QXmlSimpleReader* parser = new QXmlSimpleReader();
    XmlHttpUploadContentHandler* handler = new XmlHttpUploadContentHandler();

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

            httpUpload_->upload(handler->getPutUrl(), file_, encryptFile_);
        }
    }
    else
    {
        qDebug() << "xml response parsing failed...";
    }

    delete (handler);
    delete (parser);
}

void HttpFileUploadManager::successReceived(const QString ivAndKey)
{
    // after successfull upload, the local file must represent the hash of the get url.
    // -> rename it.

    QFileInfo localFile(file_->fileName());
    QUrl url(getUrl_);
    QString attachmentFileName;

    url.setFragment(ivAndKey);

    if( ivAndKey.size() > 0 )
    {
        url.setScheme("aesgcm");
    }

    getUrl_ = url.toString(); 
    attachmentFileName = localFile.absolutePath() + QDir::separator() +  CryptoHelper::getHashOfString(getUrl_, true);
 
    if(! QFile::rename(file_->fileName(), attachmentFileName))
    {
        qWarning() << "failed to rename file to " << attachmentFileName;
    }

    busy_ = false;

    emit fileUploadedForJidToUrl(jid_, getUrl_, "image");
}

void HttpFileUploadManager::errorReceived()
{
    busy_ = false;
}

bool HttpFileUploadManager::createAttachmentPath()
{
    QString attachmentLocation = System::getAttachmentPath();
    QDir dir(attachmentLocation);

    qDebug() << "attachment location: " << attachmentLocation;

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return dir.exists();
}

// create a path and file name with a jpg suffix
QString HttpFileUploadManager::createTargetImageName(QString source)
{
    QDateTime now(QDateTime::currentDateTimeUtc());
    uint unixTime = now.toTime_t();

    QString targetFileName = QFileInfo(source).baseName() + ".jpg";
    QString targetPath = System::getAttachmentPath() + QDir::separator() + QString::number(unixTime) + targetFileName;

    qDebug() << "target file: " << targetPath;

    return targetPath;
}

void HttpFileUploadManager::generateStatus(QString status)
{
    emit showStatus("Image Upload", status);
}
