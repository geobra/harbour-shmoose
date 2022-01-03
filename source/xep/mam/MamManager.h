#ifndef MAMMANAGER_H
#define MAMMANAGER_H

#include <QObject>
#include <QStringList>


#include <Swiften/Swiften.h>
#include <Swiften/Elements/Forwarded.h>
#include <Swiften/Elements/MAMFin.h>
#include <Swiften/Elements/ErrorPayload.h>

class Persistence;
class DownloadManager;
class LurchAdapter;

// https://xmpp.org/extensions/xep-0313.html
// requests the mam for the client jid as soon as mamNs is discovered with disco#info
// adds all bookmarks to the jids list to only query the archive one time
// requests mam for the jids in that list
class MamManager : public QObject
{
    Q_OBJECT
public:
    explicit MamManager(Persistence* persistence, LurchAdapter *lurchAdapter, QObject *parent = nullptr);
    void setupWithClient(Swift::Client* client);

    static const QString mamNs;

#ifndef UNIT_TEST
private:
#endif
    void requestArchiveForJid(const QString& jid, const QString& last = "");
    void handleMessageReceived(Swift::Message::ref message);

    void processMamMessage(std::shared_ptr<Swift::Forwarded> forwarded);
    void processFinIq(const std::string& jid, std::shared_ptr<Swift::MAMFin> payload, Swift::ErrorPayload::ref error);


    bool serverHasFeature_;
    QStringList queridJids_;

    Persistence* persistence_;
    DownloadManager* downloadManager_;
    Swift::Client* client_;
    LurchAdapter* lurchAdapter_;

public slots:
    void receiveRoomWithName(QString jid, QString name);
    void addJidforArchiveQuery(QString jid);
    void setServerHasFeatureMam(bool hasFeature);

private slots:
    void handleConnected();

};

#endif // MAMMANAGER_H
