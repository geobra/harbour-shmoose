#include "Omemo.h"
#include "System.h"
#include "XmlProcessor.h"

#include <QDir>
#include <QDebug>

extern "C" {
#include "libomemo_crypto.h"
#include "libomemo.h"
}

/*
 * https://xmpp.org/extensions/xep-0384.html
 *
 * 1. check if own id is in my device list. update if necessary. give read access to world
 * 2. check all contacts if they support omemo. must cache that list
 * 3. create and publish bundle. give read access to world
 * 4. build a session (fetch other clients bundle information)
 * 5. encrypt and send the message
 * 6. receive and decrypt a message
 */


Omemo::Omemo(QObject *parent) : QObject(parent)
{
    // create omemo path if needed
    QString omemoLocation = System::getOmemoPath();
    QDir dir(omemoLocation);

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // init omemo crypto
    omemo_default_crypto_init();

    // get the deviceListNodeName and save it for later use
    char* dlNs = nullptr;
    int ret_val = omemo_devicelist_get_pep_node_name(&dlNs);
    if (ret_val != 0)
    {
        qDebug() << "failed to get devicelist pep node name";
    }
    else
    {
        deviceListNodeName_ = QString::fromUtf8(dlNs);
        free(dlNs);
    }

    qDebug() << "Omemo::Omemo" << deviceListNodeName_;
}

void Omemo::setupWithClient(Swift::Client* client)
{
    client_ = client;

    myBareJid_ = QString::fromStdString(client_->getJID().toBare().toString());

    client_->onDataRead.connect(boost::bind(&Omemo::handleDataReceived, this, _1));

    requestDeviceList(client_->getJID());
}

void Omemo::requestDeviceList(const Swift::JID& jid)
{
    /*request
     * <iq id="87f4e888-fb43-4e4d-a1f8-09884b623f71" to="xxx@jabber-germany.de" type="get">
         <pubsub xmlns="http://jabber.org/protocol/pubsub">
          <items node="eu.siacs.conversations.axolotl.devicelist"></items>
         </pubsub>
        </iq>
     */

    /*answer
     * <iq to="xxx@jabber.ccc.de/shmoose" from="xxx@jabber-germany.de" type="result" id="87f4e888-fb43-4e4d-a1f8-09884b623f71">
         <pubsub xmlns="http://jabber.org/protocol/pubsub">
          <items node="eu.siacs.conversations.axolotl.devicelist">
               <item id="6445fd7d-6d65-4a2e-829b-9e2f6f0f0d53">
                <list xmlns="eu.siacs.conversations.axolotl">
                 <device id="1234567"></device>
                </list>
               </item>
          </items>
         </pubsub>
        </iq>
     */


    // gen the payload
    const std::string deviceListRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + deviceListNodeName_.toStdString() + "'/></pubsub>";

    Swift::RawRequest::ref requestDeviceList = Swift::RawRequest::create(Swift::IQ::Get, jid.toBare(), deviceListRequestXml, client_->getIQRouter());
    requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1));
    requestDeviceList->send();

    std::string id{requestDeviceList->getID()};
    requestedDeviceListJidIdMap_.insert(QString::fromStdString(id), QString::fromStdString(jid.toBare().toString()));
}

void Omemo::handleDeviceListResponse(const std::string& str)
{
    // implements lurch_pep_devicelist_event_handler

    qDebug() << "OMEMO: handle device list Response: " << QString::fromStdString(str);

    /*
     *  <pubsub xmlns="http://jabber.org/protocol/pubsub">
  <items node="eu.siacs.conversations.axolotl.devicelist">
   <item id="5ED52F653A338">
    <list xmlns="eu.siacs.conversations.axolotl">
     <device id="24234234"></device>
    </list>
   </item>
  </items>
 </pubsub>

    OR

    <error type=\"cancel\"><item-not-found xmlns=\"urn:ietf:params:xml:ns:xmpp-stanzas\"/></error>
     */

    /*
     * see lurch_pep_devicelist_event_handler
     * 1. check if this is our device list?!
     * -> lurch_pep_own_devicelist_request_handler
     * 2. else
     * -> omemo_devicelist_import
     * -> lurch_devicelist_process
     */


    // determine for whom this device list was requested for
    QString currentIqId = XmlProcessor::getContentInTag("iq", "id", currentNode_);

    if ( (! currentIqId.isEmpty()) && requestedDeviceListJidIdMap_.contains(currentIqId))
    {
        QString jid = requestedDeviceListJidIdMap_.value(currentIqId);
        requestedDeviceListJidIdMap_.remove(currentIqId);

        //qDebug() << "jid " << jid;
        // get the items of the request
        QString items = XmlProcessor::getChildFromNode("items", currentNode_);

        if (! items.isEmpty())
        {
            if (myBareJid_.compare(jid, Qt::CaseInsensitive) == 0)
            {
                // was a request for my device list
                ownDeviceListRequestHandler(myBareJid_, items);
            }
            else
            {
                // requested someone else device list
                qDebug() << "requested someone else device list";
            }
        }
    }
}

void Omemo::ownDeviceListRequestHandler(QString fromJid, QString items)
{
    // implements lurch_pep_own_devicelist_request_handler
    qDebug() << "Omemo::ownDeviceListRequestHandler " << fromJid << ", " << items;
}

void Omemo::handleDataReceived(Swift::SafeByteArray data)
{
    // jabber xml stream from swift to xmlnode
    std::string nodeData = Swift::safeByteArrayToString(data);
    currentNode_ = QString::fromStdString(nodeData);
}

