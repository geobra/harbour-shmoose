#include "CommandProcessor.h"
#include "Shmoose.h"

#include "System.h"
#include "CapturePicture.h"

CommandProcessor::CommandProcessor(Shmoose *shmoose, QObject *parent) : QObject(parent),
    shmoose_(shmoose), capturePicture_(new CapturePicture(System::getAttachmentPath(), this))
{
    // forward received msg from shmoose to us
    connect(shmoose_, SIGNAL(messageReceived(QString, QString)), this, SLOT(messageReceived(QString, QString)));

    // call slot if cam shot was saved to disc
    connect(capturePicture_, SIGNAL(imageSaved(int, const QString&)), this, SLOT(sendPicture(int, const QString&)));
}

void CommandProcessor::messageReceived(QString from, QString msg)
{
    // just echo everything
    if (msg.startsWith("echo", Qt::CaseInsensitive))
    {
        qDebug() << "echo";
        shmoose_->sendMessage(from, msg, "normal");
    }

    if (msg.startsWith("cam", Qt::CaseInsensitive))
    {
        shmoose_->setCurrentChatPartner(from);
        qDebug() << "cam";
        capturePicture_->shot(true);
    }
}

void CommandProcessor::sendPicture(int id, const QString& filename)
{
    qDebug() << "sendPic " << filename;
    shmoose_->sendFile(shmoose_->getCurrentChatPartner(), filename);
    shmoose_->setCurrentChatPartner("");
}
