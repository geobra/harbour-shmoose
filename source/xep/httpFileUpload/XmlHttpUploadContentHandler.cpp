#include "XmlHttpUploadContentHandler.h"

#include <iostream>
#include <QDebug>

// FIXME Error response parsing not implemented

XmlHttpUploadContentHandler::XmlHttpUploadContentHandler() : parsingSuccess_(false),
    gotSlot_(false), gotGet_(false), gotPut_(false),
    getUrl_(""), putUrl_("")
{

}

bool XmlHttpUploadContentHandler::parsedSuccessfull()
{
    return parsingSuccess_;
}

QString XmlHttpUploadContentHandler::getPutUrl()
{
    return getUrl_;
}

QString XmlHttpUploadContentHandler::getGetUrl()
{
    return putUrl_;
}


bool XmlHttpUploadContentHandler::startElement(const QString & namespaceURI, const QString & localName,
                                               const QString & qName, const QXmlAttributes & atts )
{
    //std::cout << "Read Start Tag : " << localName.toStdString()<< std::endl;
    (void) namespaceURI;
    (void) localName;
    (void) atts;

    if (qName == "slot")
    {
        gotSlot_ = true;
    }

    if (gotSlot_ == true && (qName == "get"))
    {
        gotGet_ = true;
    }

    if (gotSlot_ == true && (qName == "put"))
    {
        gotPut_ = true;
    }

    return true;
}

bool XmlHttpUploadContentHandler::characters(const QString & ch)
{
    if (gotGet_ == true)
    {
        getUrl_ = ch;
        gotGet_ = false;

        //qDebug() << "get: " << getUrl_;
    }

    if (gotPut_ == true)
    {
        putUrl_ = ch;
        gotPut_ = false;

        //qDebug() << "put: " << putUrl_;
    }

    return true;
}

bool XmlHttpUploadContentHandler::endDocument()
{
    if (getUrl_.length() > 0 && putUrl_.length() > 0)
    {
        parsingSuccess_ = true;
    }
    else
    {
        parsingSuccess_ = false;
    }

    return parsingSuccess_;
}

bool XmlHttpUploadContentHandler::fatalError (const QXmlParseException & exception)
{
    qWarning() << "Fatal error on line" << exception.lineNumber()
               << ", column" << exception.columnNumber() << ":"
               << exception.message();

    return false;
}

