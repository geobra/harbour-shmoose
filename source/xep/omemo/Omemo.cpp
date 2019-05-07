/*
 *
 * This is a port of
 * https://github.com/gkdr/lurch/blob/master/src/lurch.c
 *
 * Concept is to stay as much as possible at the original functions to make change tracking easier
 *
 */

#include "Omemo.h"
#include "System.h"

extern "C" {
#include "libomemo.h"
#include "libomemo_crypto.h"
#include "axc.h"
#include "lurch.h"
#include "xmlnode.h"
}

#include <QTimer>
#include <QDir>
#include <QDomDocument>

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStandardPaths>

#include <QDebug>

// copy from lurch.c
#define JABBER_PROTOCOL_ID "prpl-jabber"
#define LURCH_DB_SUFFIX "_db.sqlite"
#define LURCH_DB_NAME_OMEMO "omemo"
#define LURCH_DB_NAME_AXC "axc"

#include "jabber_mock.h"
#include "purple_mock.h"

extern "C" void* OmemoGetInstance() {
   return &Omemo::getInstance();
}

Omemo& Omemo::getInstance()
{
    static Omemo instance_;
    return instance_;
}

Omemo::Omemo(QObject *parent) : QObject(parent), client_(NULL), jidOfRequestedDeviceList_(""), username_(NULL)
{
    // create omemo path if needed
    QString omemoLocation = System::getOmemoPath();
    QDir dir(omemoLocation);

    // set a char* omemoDir_ for purple mock
    // FIXME use strdup
    omemoDir_ = (char*)malloc(omemoLocation.size() + 1);
    memset(omemoDir_, '\0', omemoLocation.size() + 1);
    strncpy(omemoDir_, omemoLocation.toStdString().c_str(), omemoLocation.size());

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // init omemo crypto
    omemo_default_crypto_init();

    // FIXME check version. manual check functions
    // jabber_iq_send
    // jabber_pep_request_item
    // on new lurch version to address all callbacks!
}

Omemo::~Omemo()
{
    free(username_);
    free(omemoDir_);
}

void Omemo::setupWithClient(Swift::Client *client)
{
    if (client != NULL)
    {
        client_ = client;

        // create the purple account struct
        // get jabber username
        std::string jid = client_->getJID().toBare().toString() + "/"; // trailing slash is needed by lurch
        username_ = strdup(jid.c_str());

        purpleAccount_.gc = &purpleConnection_;

        purpleConnection_.account = &purpleAccount_;
        purpleConnection_.account->username = username_;
        purpleConnection_.proto_data = &jabberStream_;

        // create the jabber stream struct
        jabberStream_.gc = &purpleConnection_;
        jabberStream_.pep = true;

        // create the purpleConveration struct
        purpleConversation_.account = &purpleAccount_;
        purpleConversation_.type = PURPLE_CONV_TYPE_IM; // FIXME CHAT for room in future release!

        client_->onDataRead.connect(boost::bind(&Omemo::handleDataReceived, this, _1));
        client_->onConnected.connect(boost::bind(&Omemo::handleConnected, this));

        QTimer::singleShot(3000, this, SLOT(shotAfterDelay()));
    }
}

void Omemo::currentChatPartner(QString jid)
{
    qDebug() << "Omemo::currentChatPartner " << jid;

    // first check if omemo session already exists
    if (isOmemoAvailableForBarJid(jid.toStdString()) == true)
    {
        emit omemoAvailableFor(jid);
    }
    else
    {
        // request !once per session! the device list of the given jid
        if (! checkedDeviceList_.contains(jid, Qt::CaseInsensitive))
        {
            // request device list. check in cb handler if jid has an devicelist entry.
            requestDeviceList(Swift::JID(jid.toStdString()));

            strncpy(partnername_, jid.toStdString().c_str(), partnerNameLength - 1);
            purpleConversation_.name = partnername_;
            lurch_conv_created_cb(&purpleConversation_);

            checkedDeviceList_.append(jid);
        }
    }
}

bool Omemo::isOmemoAvailableForBarJid(const std::string& jid)
{
    bool returnValue = false;
    axc_context * axc_ctx_p = nullptr;

    char* uname = strdup(client_->getJID().toBare().toString().c_str());

    int ret_val = lurch_axc_get_init_ctx(uname, &axc_ctx_p);
    if (ret_val == 0)
    {
        ret_val = axc_session_exists_any(jid.c_str(), axc_ctx_p);
        if (ret_val == 1)
        {
            returnValue = true;
        }
    }
    free(uname);

    return returnValue;
}

extern "C" const char *OmemoGetUname(void* omemo)
{
    return static_cast<Omemo*>(omemo)->getUname();
}

const char* Omemo::getUname()
{
    return username_;
}

extern "C" const char* OmemoGetPath(void* omemo)
{
   return static_cast<Omemo*>(omemo)->getPath();
}

const char* Omemo::getPath()
{
    return omemoDir_;
}

extern "C" void OmemoBundleRequest(void* omemo, const std::string& node)
{
   static_cast<Omemo*>(omemo)->bundleRequest(node);
}

void Omemo::bundleRequest(const std::string &node)
{
    // use iqCallBackData_ to get callback and data to use...
    // FIXME send out node and attach lurch_bundle_request_cb to the received data!

    /* <iq type='get' to='xxx@jabber.ccc.de' id='xxx@jabber.ccc.de#508164373#-1085008789'>
        <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node='eu.siacs.conversations.axolotl.bundles:508164373' max_items='1'/>
        </pubsub>
       </iq>
    */

    // FIXME
    // Swift cant send raw ready xml stanzas. So we must get the neede parts
    // out of the xml stream and construct an new iq get stanza.
    // this is just a quick regex hack and must be rewritten with xml parsing!

    QString bundleId = "";

    // get the to jid
    QRegularExpression reTo("^\\s*<\\s*iq.*to\\s*=\\s*('|\")([^'\"]+)('|\")");
    QRegularExpressionMatch toMatch = reTo.match(QString::fromStdString(node));
    if (toMatch.hasMatch()) {
        requestBundleFrom_ = toMatch.captured(2);
        qDebug() << "#" << requestBundleFrom_ << "#";
    }

    // get the stanza id
    QRegularExpression reId("^\\s*<\\s*iq.*id\\s*=\\s*('|\")([^'\"]+)('|\")");
    QRegularExpressionMatch idMatch = reId.match(QString::fromStdString(node));
    if (idMatch.hasMatch()) {
        requestBundleStanzaId_ = idMatch.captured(2);
        qDebug() << "#" << requestBundleStanzaId_ << "#";
    }

    // get the bundle id
    QRegularExpression re("eu.siacs.conversations.axolotl.bundles\\s*:\\s*(\\d+)");
    QRegularExpressionMatch bundleIdMatch = re.match(QString::fromStdString(node));
    if (bundleIdMatch.hasMatch()) {
        bundleId = bundleIdMatch.captured(1);
        qDebug() << "#" << bundleId << "#";
    }

    // gen the payload
    const std::string pubsubXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='eu.siacs.conversations.axolotl.bundles:" + bundleId.toStdString() + "' max_items='1'/></pubsub>";

    Swift::RawRequest::ref publishPep = Swift::RawRequest::create(Swift::IQ::Get,
                                                                            requestBundleFrom_.toStdString(),
                                                                            pubsubXml,
                                                                            client_->getIQRouter());

    publishPep->onResponse.connect(boost::bind(&Omemo::requestBundleHandler, this, _1));
    publishPep->send();
}

void Omemo::requestBundleHandler(const std::string& str)
{
    // str has pubsub as root node. The cb wants it to be child of iq...
    std::string bundleResponse = "<iq>" + str + "</iq>";

    xmlnode* xItems = xmlnode_from_str(bundleResponse.c_str(), -1);
    if (xItems != NULL)
    {
        lurch_bundle_request_cb(&jabberStream_,
                                requestBundleFrom_.toStdString().c_str(),
                                JABBER_IQ_SET,
                                requestBundleStanzaId_.toStdString().c_str(),
                                xItems,
                                lurchQueuedMsgPtr_
                                );

        free(xItems);
    }
}

extern "C" void OmemoStoreLurchQueuedMsgPtr(void* omemo, void* ptr)
{
   static_cast<Omemo*>(omemo)->storeLurchQueuedMsgPtr(ptr);
}

void Omemo::storeLurchQueuedMsgPtr(void* ptr)
{
    lurchQueuedMsgPtr_ = ptr;
}

void Omemo::uninstall()
{
    char* error = NULL;

    // FIXME gcc warning
    char* args[] = {"uninstall", "yes"};
    //gchar* pArgs = args[0];

    // call lurch uninstall
    lurch_cmd_func(&purpleConversation_, NULL, args, &error, NULL);

    // remove db files
    char whichAxc[] = LURCH_DB_NAME_AXC;
    char* pWhichAxc = whichAxc;

    char whichOmemo[] = LURCH_DB_NAME_OMEMO;
    char* pWhichOmemo = whichOmemo;

    std::string uname = client_->getJID().toBare().toString();

    char* axcDb = lurch_uname_get_db_fn(uname.c_str(), pWhichAxc);
    if (axcDb != NULL)
    {
        removeFile(axcDb);
        free(axcDb);
    }

    char* omemoDb = lurch_uname_get_db_fn(uname.c_str(), pWhichOmemo);
    if (omemoDb != NULL)
    {
        removeFile(omemoDb);
        free(omemoDb);
    }

    cleanupDeviceList();
}

bool Omemo::removeFile(char* file)
{
    QFile rmFile (QString::fromLocal8Bit(file));
    return rmFile.remove();
}

void Omemo::handleConnected()
{
    lurch_account_connect_cb(&purpleAccount_);
}

void Omemo::handleDataReceived(Swift::SafeByteArray data)
{
    // jabber xml stream from swift to xmlnode
    std::string nodeData = Swift::safeByteArrayToString(data);
    QString qData = QString::fromStdString(nodeData);
    //QByteArray ba = qData.toLatin1();

    //std::cout << qData.toStdString() << std::endl;

    // Message receiving and encryption is be done in MessageHandler!
    // removed the need to call lurch_xml_received_cb

    /* check for
     * <pubsub xmlns="http://jabber.org/protocol/pubsub">
         <items node="eu.siacs.conversations.axolotl.devicelist">
           <item id="1">
     */

    if ( qData.contains("eu.siacs.conversations.axolotl.devicelist", Qt::CaseInsensitive) == true )
    {
        qDebug() << "FIXME check for foreign device list and call handler!";

        QString items = getChildFromNode("items", qData);
        if (items.size() > 0)
        {
            QString from = getValueForElementInNode("iq", qData, "from");

            xmlnode* xItems = xmlnode_from_str(items.toStdString().c_str(), -1);
            if (xItems != NULL && from.contains("@", Qt::CaseInsensitive) )
            {
                lurch_pep_devicelist_event_handler(&jabberStream_, from.toStdString().c_str(), xItems);
                free(xItems);
            }
        }
    }
}

extern "C" void OmemoPepRequestItem(void* omemo, const char* uname, const char* node)
{
   static_cast<Omemo*>(omemo)->pepRequestItem(uname, node);
}

void Omemo::pepRequestItem(const char* uname, const char* node)
{
    qDebug() << "Omemo::pepRequestItem " << QString::fromLocal8Bit(node);

    // gen the payload
    const std::string pepRequest = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(node) + "' /></pubsub>";

    Swift::JID jid(uname);
    Swift::RawRequest::ref requestDeviceList = Swift::RawRequest::create(Swift::IQ::Get, jid.toBare(), pepRequest, client_->getIQRouter());
    requestDeviceList->onResponse.connect(boost::bind(&Omemo::handlePepResponse, this, _1));
    requestDeviceList->send();
}

void Omemo::handlePepResponse(const std::string& str)
{
    qDebug() << "########## OMEMO: handle PEP Response: " << QString::fromStdString(str);

    /*
         <iq lang="en" to="x@y.com/shmoose" from="xxx@jabber.ccc.de" type="result" id="63eba7ac-c3f7-450d-bc18-dfc5312a4e59">
         <pubsub xmlns="http://jabber.org/protocol/pubsub">
          <items node="eu.siacs.conversations.axolotl.devicelist">
           <item id="6027CF272F834">
            <list xmlns="eu.siacs.conversations.axolotl">
             <device id="508164373"></device>
             <device id="875461949"></device>
            </list>
           </item>
          </items>
         </pubsub>
        </iq>
     */

    // FIXME get string from lurch
    // FIXME parse to xml struct
    if (str.find("eu.siacs.conversations.axolotl.devicelist") != std::string::npos)
    {
        QString items = stripParentNodeAtElement("items", QString::fromStdString(str));
        xmlnode* node = xmlnode_from_str(items.toStdString().c_str(), -1);
        if (node != NULL)
        {
            qDebug() << "###### OMEMO: handle Devicelist";
            lurch_pep_own_devicelist_request_handler(&jabberStream_, client_->getJID().toBare().toString().c_str(), node);
            xmlnode_free(node);
        }
        else
        {
            qDebug() << "failed to convert str to xmlnode: " << QString::fromStdString(str);
        }
    }

    /*
     *<iq to='x@y.com/shmoose' from='y@x.com' type='result' id='asd'>
     * <pubsub xmlns='http://jabber.org/protocol/pubsub'>
     *  <items node='eu.siacs.conversations.axolotl.bundles:12345'>
     *   <item id='1'><bundle xmlns='eu.siacs.conversations.axolotl'>
     *    <signedPreKeyPublic signedPreKeyId='1'>sfasdfsafd</signedPreKeyPublic>
     *     <signedPreKeySignature>dgdgdsgfdfg</signedPreKeySignature>
     *      <identityKey>dgdsfgdfg</identityKey>
     *       <prekeys>
     * ...
     * </preKeyPublic></prekeys></bundle></item></items></pubsub></iq>"
     */

    if (str.find("eu.siacs.conversations.axolotl.bundles") != std::string::npos)
    {
        QString items = stripParentNodeAtElement("items", QString::fromStdString(str));
        xmlnode* node = xmlnode_from_str(items.toStdString().c_str(), -1);
        if (node != NULL)
        {
            lurch_pep_bundle_for_keytransport(&jabberStream_, client_->getJID().toBare().toString().c_str(), node);
            xmlnode_free(node);
        }
        else
        {
            qDebug() << "failed to convert str to xmlnode: " << QString::fromStdString(str);
        }
    }
}

QString Omemo::decryptMessage(const QString& msg)
{
    QString return_value = "";

    xmlnode *xNode = xmlnode_from_str(msg.toStdString().c_str(), -1);

    if (xNode != NULL)
    {
        lurch_message_decrypt(&purpleConnection_, &xNode);

        int len = 0;
        char* decrypted = xmlnode_to_str(xNode, &len);

        xmlnode_free(xNode);

        if (decrypted != NULL)
        {
            return_value = QString::fromUtf8(decrypted);
            free(decrypted);
        }
    }

    return return_value;
}

QString Omemo::encryptMessage(const QString& msg)
{
    xmlnode* xMsg = xmlnode_from_str(msg.toStdString().c_str(), -1);
    lurch_message_encrypt_im(&purpleConnection_, &xMsg);

    // FIXME check for errors from encryption and xml<->string conversion
    int len = 0;
    char* encMsg = xmlnode_to_str(xMsg, &len);
    QString encryptedMessage = QString::fromUtf8(encMsg);

    free(encMsg);
    free(xMsg);

    return encryptedMessage;
}

QString Omemo::getFirstNodeNameOfXml(const QString& xml)
{
    QString nodeName = "";

    QDomDocument d;
    if (d.setContent(xml))
    {
        nodeName = d.firstChild().nodeName();
    }

    return nodeName;
}

QString Omemo::getChildFromNode(const QString& childElement, const QString &xml)
{
    QString returnXml = "";

    QDomDocument d;
    d.setContent(xml);

    QDomElement root = d.firstChildElement();
    QDomNode n = root.firstChildElement();

    while(n.isNull() == false)
    {
        if (n.nodeName().compare(childElement, Qt::CaseInsensitive) == 0)
        {
            QDomDocument dd("");
            dd.appendChild(n);
            returnXml = dd.toString(-1);

            break;
        }
        else
        {
            QDomNode tmp;

            tmp = n.nextSibling();
            if (tmp.isNull() || tmp.nodeName().isEmpty())
            {
                // try the next child
                n = n.firstChild();
            }
            else
            {
                n= tmp;
            }
        }
    }

    return returnXml;
}

QString Omemo::stripParentNodeAtElement(const QString& childElement, const QString &xml)
{
    QString returnXml = "";

    QDomDocument d;
    d.setContent(xml);

    QDomElement element = d.documentElement();
    QDomNode n = element.firstChildElement(childElement);
    if (! n.isNull())
    {
        QDomDocument dd("");
        dd.appendChild(n);
        returnXml = dd.toString(-1);
    }

    return returnXml;
}


extern "C" void OmemoPublishRawXml(void* omemo, std::string xml)
{
   static_cast<Omemo*>(omemo)->publishPepRawXml(xml);
}

extern "C" char* getDataPathName()
{
    QString xmlLogName = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QDir::separator() + "lurch.log";
    return strdup(xmlLogName.toStdString().c_str());
}

void Omemo::publishPepRawXml(const std::string& rawXml)
{
    // gen the payload
    const std::string pubsubXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'>" + rawXml + "</pubsub>";

    Swift::RawRequest::ref publishPep = Swift::RawRequest::create(Swift::IQ::Set,
                                                                            client_->getJID().toBare(),
                                                                            pubsubXml,
                                                                            client_->getIQRouter());
    // FIXME create responder to catch the errors
    publishPep->onResponse.connect(boost::bind(&Omemo::publisedPepHandler, this, _1));

    publishPep->send();
}

void Omemo::publisedPepHandler(const std::string& str)
{
    qDebug() << "OMEMO: publisedPepHandler: " << QString::fromStdString(str);
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

    char * dl_ns = (char *) 0;

    int ret_val = omemo_devicelist_get_pep_node_name(&dl_ns);
    if (ret_val)
    {
        qDebug() << "failed to get devicelist pep node name";
    }
    else
    {
        // gen the payload
        const std::string deviceListRequestXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><items node='" + std::string(dl_ns) + "'/></pubsub>";

        Swift::RawRequest::ref requestDeviceList = Swift::RawRequest::create(Swift::IQ::Get, jid.toBare(), deviceListRequestXml, client_->getIQRouter());
        requestDeviceList->onResponse.connect(boost::bind(&Omemo::handleDeviceListResponse, this, _1));

        //requestedDeviceListForBareJid_ = QString::fromStdString(jid.toBare().toString());

        requestDeviceList->send();

        // FIXME make a map of jid and stanza id. use that in the response!
        jidOfRequestedDeviceList_ = jid.toBare().toString();

        free(dl_ns);
    }
}

void Omemo::handleDeviceListResponse(const std::string& str)
{
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


    QString items = stripParentNodeAtElement("items", QString::fromStdString(str));
    xmlnode* node = xmlnode_from_str(items.toStdString().c_str(), -1);
    if (node != NULL)
    {
        lurch_pep_devicelist_event_handler(&jabberStream_, jidOfRequestedDeviceList_.c_str(), node);
        free (node);

        if (isOmemoAvailableForBarJid(partnername_) == true)
        {
            qDebug() << "omeo available for " << partnername_;
            emit omemoAvailableFor(QString::fromLatin1(partnername_));
        }
    }
}

QString Omemo::getValueForElementInNode(const QString& node, const QString& xmlNode, const QString& elementString)
{
    QString returnValue = "";

    QDomDocument d;
    if (d.setContent(xmlNode) == true)
    {
        QDomNodeList nodeList1 = d.elementsByTagName(node);
        if (!nodeList1.isEmpty())
        {
            QDomElement element = nodeList1.at(0).toElement();
            QDomNode toNode = element.attributeNode(elementString);

            returnValue = toNode.nodeValue();
        }
    }

    return returnValue;
}

void Omemo::shotAfterDelay()
{
    //uninstall();
    //cleanupDeviceList();
}

void Omemo::cleanupDeviceList()
{
    const std::string deviceListEmptyXml = "<pubsub xmlns='http://jabber.org/protocol/pubsub'><publish node='eu.siacs.conversations.axolotl.devicelist'><item><list xmlns='eu.siacs.conversations.axolotl'></list></item></publish></pubsub>";
    Swift::RawRequest::ref emptyDeviceList = Swift::RawRequest::create(Swift::IQ::Set, client_->getJID().toBare(), deviceListEmptyXml, client_->getIQRouter());
    emptyDeviceList->send();
}
