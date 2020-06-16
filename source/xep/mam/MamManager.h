#ifndef MAMMANAGER_H
#define MAMMANAGER_H

#include <QObject>
#include <QStringList>

#include <Swiften/Swiften.h>

class Persistence;

// https://xmpp.org/extensions/xep-0313.html
// requests the mam for the client jid as soon as mamNs is discovered with disco#info
// adds all bookmarks to the jids list
// requests mam for the jids list on signal bookmarks done
class MamManager : public QObject
{
    Q_OBJECT
public:
    explicit MamManager(Persistence* persistence, QObject *parent = nullptr);
    void setupWithClient(Swift::Client* client);

    static const QString mamNs;

private:
    void requestArchiveForJid(const QString& jid);

    Persistence* persistence_;

    bool serverHasFeature_;

    QStringList queridJids_;
    Swift::Client* client_;

public slots:
    void receiveRoomWithName(QString jid, QString name);
    //void addJidforArchiveQuery(QString jid);
    void setServerHasFeatureMam(bool hasFeature);
    void requestArchives();


private slots:
    void handleConnected();


};

#endif // MAMMANAGER_H
