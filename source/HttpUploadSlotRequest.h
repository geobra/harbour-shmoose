#ifndef HTTPUPLOADSLOTREQUEST_H
#define HTTPUPLOADSLOTREQUEST_H

#include <QObject>
#include <Swiften/Queries/Request.h>
#include <Swiften/Client/Client.h>

class HttpUploadSlotRequest : public Swift::Request
{
public:
    HttpUploadSlotRequest(Swift::Client* client);
};

#endif // HTTPUPLOADSLOTREQUEST_H
