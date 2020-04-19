#include "IpHeartBeatWatcher.h"

#include <QDebug>
#include <stdio.h>
#include <errno.h>

IpHeartBeatWatcher::IpHeartBeatWatcher(QObject *parent) : QThread(parent), doWatch_(true)
{
}

IpHeartBeatWatcher::~IpHeartBeatWatcher()
{
    qDebug() << "IpHeartBeatWatcher::~IpHeartBeatWatcher";
#ifdef SFOS
    handle_ = iphb_close(handle_);
#endif
}

void IpHeartBeatWatcher::run()
{
#ifdef SFOS
    handle_ = iphb_open(NULL);
    if (handle_ != NULL)
    {
        while ( doWatch_ )
        {
            time_t time = iphb_wait(handle_, 0, (2 * IPHB_GS_WAIT_10_MINS), 1);
            printf("%ld seconds since the epoch began\n", (long)time);
            printf("%s", asctime(gmtime(&time)));

            emit triggered();
        }
    }
    else
    {
        printf("iphb_open failed! error: %s\n", strerror(errno));
    }
#else
    qDebug() << "IpHeartBeatWatcher::startWatching: only for sfos target";
#endif
}

void IpHeartBeatWatcher::stopWatching()
{
    doWatch_ = false;
}
