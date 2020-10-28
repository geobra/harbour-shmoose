#ifndef OMEMO_H
#define OMEMO_H

#include <QObject>
#include <Swiften/Swiften.h>

class Omemo : public QObject
{
    Q_OBJECT
public:
    explicit Omemo(QObject *parent = nullptr);
    void setupWithClient(Swift::Client* client);

signals:

public slots:

private:
    void requestDeviceList(const Swift::JID& jid);
    void handleDeviceListResponse(const std::string& str);


    Swift::Client* client_;
    QString deviceListNodeName_;


};

#endif // OMEMO_H
