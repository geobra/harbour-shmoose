#ifndef RECONNECTION_H
#define RECONNECTION_H

#include <QObject>
#include <QTimer>

class ReConnectionHandler : public QObject
{
    Q_OBJECT
public:
    ReConnectionHandler(unsigned int timeOut, QObject *parent = 0);
    void setActivated();

    void isConnected(bool connected);

signals:
    void canTryToReconnect();

private slots:
    void triggerIsTimedOut();

private:
    QTimer *timer_;
    const unsigned int timeOut_;

    bool activated_;
};

#endif // RECONNECTION_H
