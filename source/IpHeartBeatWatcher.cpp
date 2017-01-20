#include "IpHeartBeatWatcher.h"

#include <QDebug>
#include <stdio.h>
#include <errno.h>

#ifdef SFOS
extern "C" {
#include <iphbd/libiphb.h>
}
#endif

IpHeartBeatWatcher::IpHeartBeatWatcher(QObject *parent) : QObject(parent), doWatch_(true)
{

}

bool IpHeartBeatWatcher::startWatching()
{
#ifdef SFOS
    iphb_t handle = iphb_open(NULL);
    if (handle != NULL)
    {
        while ( doWatch_ )
        {
                time_t time = iphb_wait(handle, 0, IPHB_GS_WAIT_10_MINS, 1);
                printf("%ld seconds since the epoch began\n", (long)time);
                printf("%s", asctime(gmtime(&time)));

                emit triggered();
        }
    }
    else
    {
        printf("iphb_open failed! error: %s\n", strerror(errno));
        return false;
    }
#else
    qDebug() << "IpHeartBeatWatcher::startWatching: only for sfos target";
#endif

    return true;
}

void IpHeartBeatWatcher::stopWatching()
{
    doWatch_ = false;
}
