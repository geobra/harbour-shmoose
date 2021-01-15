#ifndef XMLPROCESSOR_H
#define XMLPROCESSOR_H

#include <QString>

class XmlProcessor
{
public:
    static QString getChildFromNode(const QString& childElement, const QString &xml);
    static QString getContentInTag(const QString& tag, const QString& param, const QString &xml);
    static QString getContentInElement(const QString& element, const QString &xml);
};

#endif // XMLPROCESSOR_H
