#include "ReConnectionHandler.h"

#include <QDebug>

ReConnectionHandler::ReConnectionHandler(unsigned int timeOut, QObject *parent) : QObject(parent),
    timer_(new QTimer()), timeOut_(timeOut), activated_(false)
{
    timer_->setInterval(timeOut_);
    timer_->setSingleShot(true);
    connect(timer_, SIGNAL(timeout()), this, SLOT(triggerIsTimedOut()));
}

void ReConnectionHandler::isConnected(bool connected)
{
    if (activated_)
    {
        if (connected == false)
        {
            qDebug() << "ReConnectionHandler::isConnected: stop timer";
            timer_->stop();
        }
        else
        {
            qDebug() << "ReConnectionHandler::isConnected: start timer";
            timer_->start();
        }
    }
}

void ReConnectionHandler::triggerIsTimedOut()
{
    if (activated_)
    {
        qDebug() << "ReConnectionHandler::triggerIsTimedOut";
        emit canTryToReconnect();
    }
}

void ReConnectionHandler::setActivated()
{
    activated_ = true;
}
