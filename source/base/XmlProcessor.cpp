#include "XmlProcessor.h"

#include <QDebug>

QString XmlProcessor::getChildFromNode(const QString& childElement, const QString &xml)
{
    QString returnXml{};

    QDomDocument d;
    d.setContent(xml);

    QDomElement root = d.firstChildElement();
    QDomNode n = root.firstChildElement();

    int loopCounter{0};

    while(n.isNull() == false)
    {
        returnXml = searchThroughChilds(n, childElement);
        if (returnXml.isEmpty() == false)
        {
            break;
        }
        else
        {
            // travers the rest of the tree
            n = n.nextSibling();
            while (n.isNull() == true && n.hasChildNodes() == true)
            {
                n = n.firstChild();
                n = n.nextSibling();

                // in case this is a really big xml struct, dont get stuck here. This should not happen!
                loopCounter++;
                if (loopCounter > 1000)
                {
                    break;
                }
            }
        }

        // in case this is a really big xml struct, dont get stuck here. This should not happen!
        loopCounter++;
        if (loopCounter > 1000)
        {
            break;
        }

    }

    return returnXml;
}

QString XmlProcessor::searchThroughChilds(QDomNode n, const QString& childElement)
{
    QString returnXml{};

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
            QDomNode sn = n.nextSibling();
            if (sn.isNull() == false)
            {
                // also search through sibling if any
                returnXml = searchThroughChilds(sn, childElement);
                if (returnXml.isEmpty() == false)
                {
                    break;
                }
            }

            n = n.firstChild();
        }
    }

    return returnXml;
}

QString XmlProcessor::getContentInTag(const QString& tag, const QString& param, const QString &xml)
{
    //qDebug() << "get: " << path << " with " << param;

    QString content = "";

    QDomDocument d;
    d.setContent(xml);

    QDomNodeList nodeList = d.elementsByTagName(tag);

    //qDebug() << "Count" << nodeList.count();

    for (int i = 0; i < nodeList.size(); i++)
    {
        if (nodeList.at(i).isElement())
        {
            //qDebug() << "is Element!";
            QDomElement first_title = nodeList.at(i).toElement().firstChildElement(param);

            if (! first_title.isNull())
            {
                content = first_title.text();
                //qDebug() << "childElement! " << first_title.text();
            }
            else
            {
                // try parameter
                QString attr = nodeList.at(i).toElement().attribute(param);

                content = attr;
                //qDebug() << "attribute! " << attr;
            }
        }

        if (content.size() > 0)
        {
            // break on the first match...
            break;
        }
    }

    return content;
}

QString XmlProcessor::getContentInElement(const QString& element, const QString &xml)
{
    //qDebug() << "get: " << path << " with " << param;

    QString content = "";

    QDomDocument d;
    d.setContent(xml);

    QDomNodeList nodeList = d.elementsByTagName(element);

    //qDebug() << "Count" << nodeList.count();

    for (int i = 0; i < nodeList.size(); i++)
    {
        if (nodeList.at(i).isElement())
        {
            //qDebug() << "is Element!";
            QDomElement first_title = nodeList.at(i).toElement();

            if (! first_title.isNull())
            {
                content = first_title.text();
                //qDebug() << "childElement! " << first_title.text();
            }
        }

        if (content.size() > 0)
        {
            // break on the first match...
            break;
        }
    }

    return content;
}
