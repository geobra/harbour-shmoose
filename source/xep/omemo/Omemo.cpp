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
#include "lurch_wrapper.h"
#include "libomemo_crypto.h"
#include "libomemo_storage.h"
#include "lurch_util.h"
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

    // determine current omemo namespace to use.
    determineNamespace(deviceListNodeName_);
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

    // only for work on the updated pep device list, which comes in as a message
    client_->onMessageReceived.connect(boost::bind(&Omemo::handleMessageReceived, this, _1));

    requestDeviceList(client_->getJID());
}

QString Omemo::getFeature()
{
    return deviceListNodeName_;
}

void Omemo::setCurrentChatPartner(const QString& jid)
{
    set_current_chat_partner(jid.toStdString().c_str());
}

void Omemo::determineNamespace(const QString& nsDl)
{
    QString seperator = ".";
    if (nsDl.indexOf(seperator) == -1)
    {
        seperator = ":";
    }

    auto nsList = nsDl.split(seperator);

    for (auto i = 0; i < nsList.size() - 1; i++)
    {
        namespace_ += nsList.at(i);
        if (i < nsList.size() - 2)
        {
            namespace_ += seperator;
        }
    }
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
    //qDebug() << "Omemo::sendAsPepStanza" << stanza;

    // check what to send via pep.
    // can be a bundle or a device list.
    // set callback accordingly
    /*
     * <publish node="eu.siacs.conversations.axolotl.bundles:123456"><item><bundle xmlns="eu.siacs.conversations.axolotl">...
     * <publish node='eu.siacs.conversations.axolotl.devicelist'><item><list xmlns='eu.siacs.conversations.axolotl'>...
     */

    QString publishNodeType = XmlProcessor::getContentInTag("publish", "node", stanza);

    std::string pubsub = "<pubsub xmlns='http://jabber.org/protocol/pubsub'>" + std::string(stz) + "</pubsub>";

    // set empty ("") receiver so that the pep gets distributed to all pubsubs
    Swift::RawRequest::ref publishPep = Swift::RawRequest::create(Swift::IQ::Set, "", pubsub, client_->getIQRouter());
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
    lurch_message_encrypt_im_wrap(nullptr, &node);

    int len = 0;
    char* cryptedNode = xmlnode_to_str(node, &len);
    xmlnode_free(node);

    if (len > 0)
    {
        std::string returnStr{cryptedNode};
        free(cryptedNode);

        return returnStr;
    }
    else
    {
        return "";
    }
}

int Omemo::decryptMessageIfEncrypted(Swift::Message::ref aMessage)
{
    int returnValue{0};

    // check if received aMessage is encrypted
    QString qMsg = getSerializedStringFromMessage(aMessage);
    if (isEncryptedMessage(qMsg))
    {
        std::string dmsg = messageDecrypt(qMsg.toStdString());
        QString decryptedMessage{QString::fromStdString(dmsg)};

        if (decryptedMessage.isEmpty())
        {
            // something went wrong on the decryption.
            returnValue = 2;
        }
        else
        {
            QString body = XmlProcessor::getContentInElement("body", decryptedMessage);
            aMessage->setBody(body.toStdString());
            returnValue = 0;
        }
    }
    else
    {
        // was not an encrypted msg
        returnValue = 1;
    }

    return returnValue;
}

std::string Omemo::messageDecrypt(const std::string& message)
{
    xmlnode* node = xmlnode_from_str(message.c_str(), -1);
    lurch_message_decrypt_wrap(nullptr, &node);

    int len = 0;
    char* decryptedNode = xmlnode_to_str(node, &len);    
    xmlnode_free(node);

    if (len > 0)
    {
        std::string returnStr{decryptedNode};
        free(decryptedNode);

        return returnStr;
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
    lurch_bundle_request_cb_wrap(&jabberStream, jid.toBare().toString().c_str(), JABBER_IQ_SET, id.c_str(), node, qMsg);

    xmlnode_free(node);
}

void Omemo::pepBundleForKeytransport(const std::string from, const std::string &items)
{
    xmlnode* itemsNode = xmlnode_from_str(items.c_str(), -1);
    lurch_pep_bundle_for_keytransport_wrap(&jabberStream, from.c_str(), itemsNode);

    xmlnode_free(itemsNode);
}

void Omemo::handleDeviceListResponse(const Swift::JID jid, const std::string& str)
{
    // implements lurch_pep_devicelist_event_handler

    std::string bareJidStr = jid.toBare().toString();
    QString qJid = QString::fromStdString(bareJidStr);
    //qDebug() << "OMEMO: handle device list Response. Jid: " << qJid << ". list: " << QString::fromStdString(str);

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
        lurch_pep_own_devicelist_request_handler_wrap(&jabberStream, uname_, itemsNode);

        xmlnode_free(itemsNode);
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
                if(lurch_devicelist_process_wrap(uname_, dl_in_p, &jabberStream) != 0)
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

QString Omemo::getSerializedStringFromMessage(Swift::Message::ref msg)
{
    Swift::FullPayloadSerializerCollection serializers_;
    Swift::XMPPSerializer xmppSerializer(&serializers_, Swift::ClientStreamType, true);
    Swift::SafeByteArray sba = xmppSerializer.serializeElement(msg);

    return QString::fromStdString(Swift::safeByteArrayToString(sba));
}

bool Omemo::exchangePlainBodyByOmemoStanzas(Swift::Message::ref msg)
{
    bool returnValue{false};

    QString qMsg = getSerializedStringFromMessage(msg);
    if (qMsg.isEmpty() == false)
    {
        std::string cryptMessage = messageEncryptIm(qMsg.toStdString());
        if (cryptMessage.empty() == false) // no fatal error. Either msg is original, or omemo encrypted
        {
            QString encryptedPayload = XmlProcessor::getChildFromNode("encrypted", QString::fromStdString(cryptMessage));
            if (encryptedPayload.isEmpty() == false)
            {
                Swift::RawXMLPayload::ref encPayload = std::make_shared<Swift::RawXMLPayload>(
                            encryptedPayload.toStdString()
                            + "<encryption xmlns=\"urn:xmpp:eme:0\" namespace=\"" + namespace_.toStdString() + "\" name=\"OMEMO\" /><store xmlns=\"urn:xmpp:hints\" />"
                        );
                msg->addPayload(encPayload);

                msg->removePayloadOfSameType(std::make_shared<Swift::Body>());

                returnValue = true;
            }
        }
    }

    return returnValue;
}

void Omemo::callLurchCmd(const std::vector<std::string>& sl)
{
    std::vector<char*> cstrings;
    cstrings.reserve(sl.size());

    for(size_t i = 0; i < sl.size(); ++i)
    {
        cstrings[i] = const_cast<char*>(sl[i].data());
    }

    char** p_str_array = cstrings.data();

    lurch_cmd_func_wrap(nullptr, "", p_str_array, nullptr, nullptr);
}

bool Omemo::isOmemoUser(const QString& bareJid)
{
    bool returnValue{false};

    char * db_fn_omemo = lurch_util_uname_get_db_fn(client_->getJID().toBare().toString().c_str(), LURCH_DB_NAME_OMEMO);
    omemo_devicelist * dl_p{nullptr};

    // determine if recipient is omemo user
    int ret_val = omemo_storage_user_devicelist_retrieve(bareJid.toStdString().c_str(), db_fn_omemo, &dl_p);
    if (ret_val)
    {
        returnValue = false;
    }
    else
    {
        returnValue = true;
    }

    return returnValue;
}

void Omemo::handleMessageReceived(Swift::Message::ref message)
{
#if 0
    <message from='juliet@capulet.lit'
             to='romeo@montague.lit'
             type='headline'
             id='update_01'>
      <event xmlns='http://jabber.org/protocol/pubsub#event'>
        <items node='urn:xmpp:omemo:1:devices'>
          <item id='current'>
            <devices xmlns='urn:xmpp:omemo:1'>
              <device id='12345' />
              <device id='4223' label='Gajim on Ubuntu Linux' />
            </devices>
          </item>
        </items>
      </event>
    </message>

            <message xmlns="jabber:client" to="user2@localhost/shmooseDesktop" from="user1@localhost" type="headline">
             <event xmlns="http://jabber.org/protocol/pubsub#event">
              <items node="eu.siacs.conversations.axolotl.devicelist">
               <item id="64BBE01EA5421">
                <list xmlns="eu.siacs.conversations.axolotl">
                 <device id="226687003"></device>
                </list>
               </item>
              </items>
             </event>
             <addresses xmlns="http://jabber.org/protocol/address">
              <address jid="user1@localhost/8832136518768075312" type="replyto"></address>
             </addresses>
            </message>
#endif

    QString msg = getSerializedStringFromMessage(message);

    QString itemsNodeType = XmlProcessor::getContentInTag("items", "node", msg);

    if(itemsNodeType.contains("devicelist", Qt::CaseInsensitive) == true)
    {
        QString from = XmlProcessor::getContentInTag("message", "from", msg);
        QString items = XmlProcessor::getChildFromNode("items", msg);
        xmlnode* xItems = xmlnode_from_str(items.toStdString().c_str(), -1);

        if (! items.isEmpty())
        {
            qDebug() << "Omemo::handleMessageReceived. items: " << items;
            lurch_pep_devicelist_event_handler_wrap(&jabberStream, from.toStdString().c_str(), xItems);

            emit signalReceivedDeviceListOfJid(from);
        }

        xmlnode_free(xItems);
    }
}
