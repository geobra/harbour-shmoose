#ifndef MAMMANAGER_H
#define MAMMANAGER_H

#include <QObject>
#include <QStringList>

#include <Swiften/Swiften.h>

class Persistence;
class DownloadManager;

// https://xmpp.org/extensions/xep-0313.html
// requests the mam for the client jid as soon as mamNs is discovered with disco#info
// adds all bookmarks to the jids list to only query the archive one time
// requests mam for the jids in that list
class MamManager : public QObject
{
    Q_OBJECT
public:
    explicit MamManager(Persistence* persistence, QObject *parent = nullptr);
    void setupWithClient(Swift::Client* client);

    static const QString mamNs;

#ifndef UNIT_TEST
private:
#endif
    void requestArchiveForJid(const QString& jid, const QString& last = "");
    void handleDataReceived(Swift::SafeByteArray data);

    void processMamMessage(const QString& qData);
    void processFinIq(const QString& iq);


    bool serverHasFeature_;
    QStringList queridJids_;

    Persistence* persistence_;
    DownloadManager* downloadManager_;
    Swift::Client* client_;

public slots:
    void receiveRoomWithName(QString jid, QString name);
    void addJidforArchiveQuery(QString jid);
    void setServerHasFeatureMam(bool hasFeature);

private slots:
    void handleConnected();

};

#endif // MAMMANAGER_H
