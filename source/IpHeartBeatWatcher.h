#ifndef IPHEARTBEATWATCHER_H
#define IPHEARTBEATWATCHER_H

#include <QObject>

class IpHeartBeatWatcher : public QObject
{
	Q_OBJECT
public:
	IpHeartBeatWatcher(QObject *parent = 0);

    bool startWatching();
    void stopWatching();

signals:
	void triggered();

private:
    bool doWatch_;

};

#endif
