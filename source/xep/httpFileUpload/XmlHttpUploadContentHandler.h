#ifndef XMLHTTPUPLOADCONTENTHANDLER_H
#define XMLHTTPUPLOADCONTENTHANDLER_H

#include <QXmlDefaultHandler>

class XmlHttpUploadContentHandler : public QXmlDefaultHandler
{
public:
    XmlHttpUploadContentHandler();

    bool parsedSuccessfull();
    QString getPutUrl();
    QString getGetUrl();

private:
    bool startElement(const QString & namespaceURI, const QString & localName,
                      const QString & qName, const QXmlAttributes & atts );

    bool characters(const QString & ch);

    bool endDocument();

    bool fatalError (const QXmlParseException & exception);

    bool parsingSuccess_;

    bool gotSlot_;
    bool gotGet_;
    bool gotPut_;

    QString getUrl_;
    QString putUrl_;

};

#endif // XMLHTTPUPLOADCONTENTHANDLER_H
