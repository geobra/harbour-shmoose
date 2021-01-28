#ifndef XMLPROCESSOR_H
#define XMLPROCESSOR_H

#include <QString>
#include <QDomComment>

class XmlProcessor
{
public:
    static QString getChildFromNode(const QString& childElement, const QString &xml);
    static QString getContentInTag(const QString& tag, const QString& param, const QString &xml);
    static QString getContentInElement(const QString& element, const QString &xml);

private:
    static QString searchThroughChilds(QDomNode n, const QString& childElement);
};

#endif // XMLPROCESSOR_H
