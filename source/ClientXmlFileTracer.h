#ifndef CLIENTXMLFILETRACER_H
#define CLIENTXMLFILETRACER_H

#include <Swiften/Client/CoreClient.h>
#include <Swiften/Client/XMLBeautifier.h>
#include <Swiften/Base/SafeByteArray.h>

#include <fstream>

class ClientXmlFileTracer
{
public:
    ClientXmlFileTracer(Swift::CoreClient* client, const std::string& file);
    ~ClientXmlFileTracer();

private:
    void printData(char direction, const Swift::SafeByteArray& data);
    void printLine(char c);

private:
    std::ofstream logFile_;
    bool logToFile_;

    Swift::XMLBeautifier *beautifier;

    boost::bsignals::scoped_connection onDataReadConnection;
    boost::bsignals::scoped_connection onDataWrittenConnection;
};

#endif // CLIENTXMLFILETRACER_H
