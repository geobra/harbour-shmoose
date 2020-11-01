#include "DiscoInfoHandler.h"
#include "HttpFileUploadManager.h"
#include "MamManager.h"

#include <QDebug>

DiscoInfoHandler::DiscoInfoHandler(HttpFileUploadManager *httpFileUploadManager, MamManager *mamManager, QObject *parent) : QObject(parent),
    httpFileUploadManager_(httpFileUploadManager), mamManager_(mamManager),
    client_(nullptr), danceFloor_()
{

}

DiscoInfoHandler::~DiscoInfoHandler()
{
    cleanupDiscoServiceWalker();
}

void DiscoInfoHandler::setupWithClient(Swift::Client* client)
{
    if (client != nullptr)
    {
        client_ = client;

        // request the discoInfo from server
        std::shared_ptr<Swift::DiscoServiceWalker> topLevelInfo(
                    new Swift::DiscoServiceWalker(Swift::JID(client_->getJID().getDomain()), client_->getIQRouter()));
        topLevelInfo->onServiceFound.connect(boost::bind(&DiscoInfoHandler::handleDiscoServiceWalker, this, _1, _2));
        topLevelInfo->beginWalk();
        danceFloor_.append(topLevelInfo);
    }
}

void DiscoInfoHandler::handleDiscoServiceWalker(const Swift::JID & jid, std::shared_ptr<Swift::DiscoInfo> info)
{
#if 0
    qDebug() << "Shmoose::handleDiscoWalkerService for '" << QString::fromStdString(jid.toString()) << "'.";
    for(auto feature : info->getFeatures())
    {
        qDebug() << "Shmoose::handleDiscoWalkerService feature '" << QString::fromStdString(feature) << "'.";
    }
#endif

    const std::string httpUpload = "urn:xmpp:http:upload";

    if (info->hasFeature(httpUpload))
    {
        qDebug() << QString::fromStdString(jid.toString()) << " has feature urn:xmpp:http:upload";
        httpFileUploadManager_->setServerHasFeatureHttpUpload(true);
        httpFileUploadManager_->setUploadServerJid(jid);
        emit serverHasHttpUpload_(true);

        foreach (Swift::Form::ref form, info->getExtensions())
        {
            if (form)
            {
                if ((*form).getFormType() == httpUpload)
                {
                    Swift::FormField::ref formField = (*form).getField("max-file-size");
                    if (formField)
                    {
                        unsigned int maxFileSize = std::stoi((*formField).getTextSingleValue());
                        //qDebug() << QString::fromStdString((*formField).getName()) << " val: " << maxFileSize;
                        httpFileUploadManager_->setMaxFileSize(maxFileSize);
                    }
                }
            }
        }
    }

    if (info->hasFeature(MamManager::mamNs.toStdString()))
    {
        qDebug() << "### " << QString::fromStdString(jid.toString()) << " has " << MamManager::mamNs;
        //mamManager_->setServerHasFeatureMam(true);
        emit serverHasMam_(true);
    }
}

void DiscoInfoHandler::cleanupDiscoServiceWalker()
{
    for(auto walker : danceFloor_)
    {
        walker->endWalk();
    }

    danceFloor_.clear();
}
