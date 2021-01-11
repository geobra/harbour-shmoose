#include "Omemo.h"
#include "System.h"
#include "XmlProcessor.h"
#include "Settings.h"
#include "System.h"
#include "RawRequestWithFromJid.h"
#include "RawRequestBundle.h"
#include "CToCxxProxy.h"

#include <QDir>
#include <QDomDocument>
#include <QDebug>

#include <stdlib.h>

extern "C"
{
#include <purple.h>
#include "lurch_prep.h"
#include "libomemo_crypto.h"
}

/*
 * https://www.ietf.org/rfc/rfc3920.txt
 * https://xmpp.org/extensions/xep-0384.html
 *
 * 1. check if own id is in my device list. update if necessary. give read access to world
 * 2. check all contacts if they support omemo. subscribe to 'eu.siacs.conversations.axolotl.devicelist' (new: 'urn:xmpp:omemo:1:devices'). must cache that list
 * 3. create and publish bundle. give read access to world
 * 4. build a session (fetch other clients bundle information)
 * 5. encrypt and send the message
 * 6. receive and decrypt a message
 */

// FIXME! implement lurch_pep_devicelist_event_handler

Omemo::Omemo(QObject *parent) : QObject(parent)
{
    // lurch_plugin_load

    set_omemo_dir(System::getOmemoPath().toStdString().c_str());
    CToCxxProxy::getInstance().setOmemoPtr(this);

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

    //qDebug() << "Omemo::Omemo" << deviceListNodeName_;
}

Omemo::~Omemo()
{
    omemo_default_crypto_teardown();
    free(uname_);
}

void Omemo::setupWithClient(Swift::Client* client)
{
    client_ = client;

    myBareJid_ = QString::fromStdString(client_->getJID().toBare().toString());
    uname_ = strdup(myBareJid_.toStdString().c_str());

    set_fqn_name(client_->getJID().toString().c_str());

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

    RawRequestWithFromJid::ref requestDeviceList = RawRequestWithFromJid::create(Swift::IQ::Get, jid.toBare(), deviceListRequestXml, client_->getIQRouter());
    requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1, _2));
    requestDeviceList->send();
}

void Omemo::sendAsPepStanza(char* stz)
{
    QString stanza = QString::fromLatin1(stz);
    qDebug() << "Omemo::sendAsPepStanza" << stanza;

    // FIXME check what to send via pep.
    // can be a bundle or a device list.
    // set callback accordingly
    /*
     * <publish node="eu.siacs.conversations.axolotl.bundles:123456"><item><bundle xmlns="eu.siacs.conversations.axolotl">...
     * <publish node='eu.siacs.conversations.axolotl.devicelist'><item><list xmlns='eu.siacs.conversations.axolotl'>...
     */

    QString publishNodeType = XmlProcessor::getContentInTag("publish", "node", stanza);

    std::string pubsub = "<pubsub xmlns='http://jabber.org/protocol/pubsub'>" + std::string(stz) + "</pubsub>";
    Swift::RawRequest::ref publishPep = Swift::RawRequest::create(Swift::IQ::Set, uname_, pubsub, client_->getIQRouter());
    if (publishNodeType.contains("bundle", Qt::CaseInsensitive))
    {
        publishPep->onResponse.connect(boost::bind(&Omemo::publishedBundle, this, _1));
    }
    else if (publishNodeType.contains("devicelist", Qt::CaseInsensitive))
    {
        publishPep->onResponse.connect(boost::bind(&Omemo::publishedDeviceList, this, _1));
    }
    else
    {
        qDebug() << "Error: unknown pep to send: " << stanza;
    }

    publishPep->send();
}

void Omemo::sendRawMessageStanza(char* stz)
{
    QString stanza = QString::fromLatin1(stz);
    emit rawMessageStanzaForSending(stanza);
}

void Omemo::sendBundleRequest(char* node, void* q_msg)
{
    //node contains
    /* <iq type='get' to='xxx@jabber.ccc.de' id='xxx@jabber.ccc.de#508164373#-1085008789'>
        <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node='eu.siacs.conversations.axolotl.bundles:508164373' max_items='1'/>
        </pubsub>
       </iq>
    */

    QString xmlNode = QString::fromLatin1(node);
    QString toJid = XmlProcessor::getContentInTag("iq", "to", xmlNode);
    QString pubSub = XmlProcessor::getChildFromNode("pubsub", xmlNode);
    QString bundleString = XmlProcessor::getContentInTag("items", "node", xmlNode);
    QStringList bundleList = bundleString.split(":");
    if (bundleList.size() == 2)
    {
        QString bundleId = bundleList.at(1);

        RawRequestBundle::ref publishPep = RawRequestBundle::create(Swift::IQ::Get,
                                                                                toJid.toStdString(),
                                                                                pubSub.toStdString(),
                                                                                client_->getIQRouter(),
                                                                                bundleId.toStdString(),
                                                                                q_msg
                                                                            );

        publishPep->onResponse.connect(boost::bind(&Omemo::requestBundleHandler, this, _1, _2, _3, _4));
        publishPep->send();
    }
}

void Omemo::createAndSendBundleRequest(char* sender, char* bundle)
{
    const std::string bundleRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(bundle) + "'/></pubsub>";
    RawRequestWithFromJid::ref requestDeviceList = RawRequestWithFromJid::create(Swift::IQ::Get, std::string(sender), bundleRequestXml, client_->getIQRouter());
    requestDeviceList->onResponse.connect(boost::bind(&Omemo::pepBundleForKeytransport, this, _1, _2));
    requestDeviceList->send();
}

std::string Omemo::messageEncryptIm(const std::string msg)
{
    xmlnode* node = xmlnode_from_str(msg.c_str(), -1);
    lurch_message_encrypt_im(nullptr, &node);

    int len = 0;
    char* cryptedNode = xmlnode_to_str(node, &len);

    if (len > 0)
    {
        return std::string(cryptedNode);
    }
    else
    {
        return "";
    }
}

std::string Omemo::messageDecrypt(const std::string& message)
{
    xmlnode* node = xmlnode_from_str(message.c_str(), -1);
    lurch_message_decrypt(nullptr, &node);

    int len = 0;
    char* decryptedNode = xmlnode_to_str(node, &len);

    if (len > 0)
    {
        return std::string(decryptedNode);
    }
    else
    {
        return "";
    }
}

void Omemo::requestBundleHandler(const Swift::JID& jid, const std::string& bundleId, void* qMsg, const std::string& str)
{
    // str has pubsub as root node. The cb wants it to be child of iq...
    std::string bundleResponse = "<iq>" + str + "</iq>";

    std::string id = "foo#bar#" + bundleId;
    xmlnode* node = xmlnode_from_str(bundleResponse.c_str(), -1);
    lurch_bundle_request_cb(&jabberStream, jid.toBare().toString().c_str(), JABBER_IQ_SET, id.c_str(), node, qMsg);
}

void Omemo::pepBundleForKeytransport(const std::string from, const std::string &items)
{
    xmlnode* itemsNode = xmlnode_from_str(items.c_str(), -1);
    lurch_pep_bundle_for_keytransport(&jabberStream, from.c_str(), itemsNode);

    xmlnode_free(itemsNode);
}

void Omemo::handleDeviceListResponse(const Swift::JID jid, const std::string& str)
{
    // implements lurch_pep_devicelist_event_handler

    std::string bareJidStr = jid.toBare().toString();
    QString qJid = QString::fromStdString(bareJidStr);
    qDebug() << "OMEMO: handle device list Response. Jid: " << qJid << ". list: " << QString::fromStdString(str);

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


    // get the items of the request
    QString items = XmlProcessor::getChildFromNode("items", QString::fromStdString(str));

    // call this methods even with empty 'items'! Its an init path to the bundleList.
    if (myBareJid_.compare(qJid, Qt::CaseInsensitive) == 0)
    {
        // was a request for my device list
        xmlnode* itemsNode = xmlnode_from_str(items.toStdString().c_str(), -1);
        lurch_pep_own_devicelist_request_handler(&jabberStream, uname_, itemsNode);
    }
    else
    {
        // requested someone else device list
        if ( (!items.isEmpty()) && items.startsWith("<items"))
        {
            qDebug() << "requested someone else device list: " << QString::fromStdString(bareJidStr) << ": " << items;

            omemo_devicelist* dl_in_p = nullptr;
            char* pItems = strdup(items.toStdString().c_str());

            int ret = omemo_devicelist_import(pItems, bareJidStr.c_str(), &dl_in_p);
            if ( ret == 0)
            {
                if(lurch_devicelist_process(uname_, dl_in_p, &jabberStream) != 0)
                {
                    qDebug() << "failed to process devicelist";
                }
                else
                {
                    emit signalReceivedDeviceListOfJid(qJid);
                }

                omemo_devicelist_destroy(dl_in_p);
            }
            else
            {
                qDebug() << "failed to import devicelist: " << ret;
            }
            free(pItems);
        }
    }
}

void Omemo::publishedDeviceList(const std::string& str)
{
    // FIXME check if there was an error on device list publishing
    qDebug() << "OMEMO: publishedDeviceList: " << QString::fromStdString(str);
}

void Omemo::publishedBundle(const std::string& str)
{
    // FIXME check if there was an error on bundle publishing
    qDebug() << "OMEMO: publishedBundle: " << QString::fromStdString(str);
}

bool Omemo::isEncryptedMessage(const QString& xmlNode)
{
    bool returnValue = false;

    QDomDocument d;
    if (d.setContent(xmlNode) == true)
    {
        QDomNodeList nodeList = d.elementsByTagName("message");
        if (!nodeList.isEmpty())
        {
            //qDebug() << "found msg";
            QDomNodeList encList = d.elementsByTagName("encrypted");
            if (!encList.isEmpty())
            {
                //qDebug() << "found enc";
                returnValue = true;
            }
        }
    }

    return returnValue;
}

// FIXME not just request the device list in this slot but also subscribe to the pep node!
void Omemo::slotRequestDeviceList(QString humanBareJid)
{
    qDebug() << "request device list for " << humanBareJid;
    requestDeviceList(Swift::JID(humanBareJid.toStdString()));
}
