#ifndef IPHEARTBEATWATCHER_H
#define IPHEARTBEATWATCHER_H

#ifdef SFOS
extern "C" {
#include <iphbd/libiphb.h>
}
#endif

#include <QThread>
#include <QObject>

class IpHeartBeatWatcher : public QThread
{
    Q_OBJECT
public:
    IpHeartBeatWatcher(QObject *parent = 0);
    ~IpHeartBeatWatcher();

    void run();
    void stopWatching();

signals:
    void triggered();

private:
    bool doWatch_;

#ifdef SFOS
    iphb_t handle_;
#endif

};

#endif
