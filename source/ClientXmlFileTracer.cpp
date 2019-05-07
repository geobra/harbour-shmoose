#include "ClientXmlFileTracer.h"

#include <iostream>
#include <boost/bind.hpp>
#include <Swiften/Base/Platform.h>

ClientXmlFileTracer::ClientXmlFileTracer(Swift::CoreClient *client, const std::string &file) : logToFile_(false)
{
    logFile_.open(file , std::ios_base::out | std::ios_base::trunc);

    if (logFile_.rdstate() == 0)
    {
        logToFile_ = true;
    }

    beautifier = new Swift::XMLBeautifier(true, false);

    onDataReadConnection = client->onDataRead.connect(boost::bind(&ClientXmlFileTracer::printData, this, '<', _1));
    onDataWrittenConnection = client->onDataWritten.connect(boost::bind(&ClientXmlFileTracer::printData, this, '>', _1));
}

ClientXmlFileTracer::~ClientXmlFileTracer()
{
    if (logToFile_ == true)
    {
        logFile_.flush();
        logFile_.close();
    }

    delete beautifier;
}

void ClientXmlFileTracer::printData(char direction, const Swift::SafeByteArray& data)
{
    printLine(direction);

    std::string log = beautifier->beautify(Swift::byteArrayToString(Swift::ByteArray(data.begin(), data.end())));
    if (logToFile_ == true)
    {
        logFile_ << log << std::endl;
    }
    else
    {
        std::cerr << log << std::endl;
    }
}

void ClientXmlFileTracer::printLine(char c)
{
    std::ostringstream buf;

    for (unsigned int i = 0; i < 80; ++i)
    {
        buf << c;
    }
    buf << std::endl;

    if (logToFile_ == true)
    {
        logFile_ << buf.str();
    }
    else
    {
        std::cerr << buf.str();
    }
}
