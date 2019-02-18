#ifndef XMPPPINGCONTROLLER_H
#define XMPPPINGCONTROLLER_H

#include <string>

namespace Swift
{
class Client;
}

class XmppPingController
{
public:
    XmppPingController();

    void setupWithClient(Swift::Client* client);
    void doPing();

private:
    void handlePingResponse(const std::string response);

    Swift::Client* client_;

};

#endif // XMPPPINGCONTROLLER_H
