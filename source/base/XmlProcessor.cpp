#include "XmlProcessor.h"

#include <QDomComment>

QString XmlProcessor::getChildFromNode(const QString& childElement, const QString &xml)
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
