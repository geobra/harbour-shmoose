#ifndef IPHEARTBEATWATCHER_H
#define IPHEARTBEATWATCHER_H

#ifdef SFOS
extern "C" {
#include <iphbd/libiphb.h>
}
#endif

#include <QObject>
#include <poll.h>

class IpHeartBeatWatcher : public QObject
{
	Q_OBJECT
public:
	IpHeartBeatWatcher(QObject *parent = 0);
    ~IpHeartBeatWatcher();

    bool startWatching();
    void stopWatching();

signals:
	void triggered();

private:
    bool openIpHb();
    void closeIpHb();

    bool doWatch_;

    pollfd ipHbPollFd_;

#ifdef SFOS
    iphb_t handle_;
#endif

};

#endif
