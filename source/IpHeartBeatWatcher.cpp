#include "IpHeartBeatWatcher.h"

#include <QTime>
#include <QDebug>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <unistd.h>

IpHeartBeatWatcher::IpHeartBeatWatcher(QObject *parent) : QObject(parent), doWatch_(true)
{
}

IpHeartBeatWatcher::~IpHeartBeatWatcher()
{
    stopWatching();
}

bool IpHeartBeatWatcher::openIpHb()
{
    bool returnValue = false;

#ifdef SFOS
    handle_ = iphb_open(NULL);
    if (handle_ != NULL)
    {
        time_t time = iphb_wait(handle_, 0, (2 * IPHB_GS_WAIT_10_MINS), 0);
        //printf("%ld seconds since the epoch began\n", (long)time);
        printf("iphb_wait: %s", asctime(gmtime(&time)));

        ipHbPollFd_.fd = iphb_get_fd(handle_);
        ipHbPollFd_.events = POLLIN;

        returnValue = true;
    }
    else
    {
        printf("iphb_open failed! error: %s\n", strerror(errno));
    }
#endif

    return returnValue;
}

void IpHeartBeatWatcher::closeIpHb()
{
#ifdef SFOS
    handle_ = iphb_close(handle_);
#endif
}

bool IpHeartBeatWatcher::startWatching()
{
#ifdef SFOS
    bool open = openIpHb();

    if (open == true)
    {
	    while ( doWatch_ )
	    {
		    int pollReturn = poll(&ipHbPollFd_, 1, 3000);

		    if (pollReturn > 0)
		    {
			    emit triggered();

			    //Debug() << QTime::currentTime().toString();

			    // read away the data is not enough here...
			    closeIpHb();
			    openIpHb();
		    }
		    else
		    {
			    //qDebug() << "timeout " << pollReturn;
		    }
	    }
	    closeIpHb();
    }

#else
    qDebug() << "IpHeartBeatWatcher::startWatching: only for sfos target";
    return false;
#endif

    return true;
}

void IpHeartBeatWatcher::stopWatching()
{
    doWatch_ = false;
}
