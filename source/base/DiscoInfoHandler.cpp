#include "DiscoInfoHandler.h"
#include "HttpFileUploadManager.h"

#include <QDebug>

DiscoInfoHandler::DiscoInfoHandler(HttpFileUploadManager *httpFileUploadManager, QObject *parent) : QObject(parent),
    httpFileUploadManager_(httpFileUploadManager), client_(NULL), discoItemReq_(NULL), danceFloor_()
{

}

DiscoInfoHandler::~DiscoInfoHandler()
{
    cleanupDiscoServiceWalker();
}

void DiscoInfoHandler::setupWithClient(Swift::Client* client)
{
    if (client != NULL)
    {
        client_ = client;

        // request the discoInfo from server
        boost::shared_ptr<Swift::DiscoServiceWalker> topLevelInfo(
                    new Swift::DiscoServiceWalker(Swift::JID(client_->getJID().getDomain()), client_->getIQRouter()));
        topLevelInfo->onServiceFound.connect(boost::bind(&DiscoInfoHandler::handleDiscoServiceWalker, this, _1, _2));
        topLevelInfo->beginWalk();
        danceFloor_.append(topLevelInfo);

        // find additional items on the server
        discoItemReq_ = Swift::GetDiscoItemsRequest::create(Swift::JID(client_->getJID().getDomain()), client_->getIQRouter());
        discoItemReq_->onResponse.connect(boost::bind(&DiscoInfoHandler::handleServerDiscoItemsResponse, this, _1, _2));
        discoItemReq_->send();
    }
}

void DiscoInfoHandler::handleDiscoServiceWalker(const Swift::JID & jid, boost::shared_ptr<Swift::DiscoInfo> info)
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
        qDebug() << "has feature urn:xmpp:http:upload";
        httpFileUploadManager_->setServerHasFeatureHttpUpload(true);
        httpFileUploadManager_->setUploadServerJid(jid);

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
}

void DiscoInfoHandler::cleanupDiscoServiceWalker()
{
    for(auto walker : danceFloor_)
    {
        walker->endWalk();
    }

    danceFloor_.clear();
}

void DiscoInfoHandler::handleServerDiscoItemsResponse(boost::shared_ptr<Swift::DiscoItems> items, Swift::ErrorPayload::ref error)
{
    //qDebug() << "Shmoose::handleServerDiscoItemsResponse";
    if (!error)
    {
        for(auto item : items->getItems())
        {
            //qDebug() << "Item '" << QString::fromStdString(item.getJID().toString()) << "'.";
            boost::shared_ptr<Swift::DiscoServiceWalker> itemInfo(
                        new Swift::DiscoServiceWalker(Swift::JID(client_->getJID().getDomain()), client_->getIQRouter()));
            itemInfo->onServiceFound.connect(boost::bind(&DiscoInfoHandler::handleDiscoServiceWalker, this, _1, _2));
            itemInfo->beginWalk();
            danceFloor_.append(itemInfo);
        }
    }
}
