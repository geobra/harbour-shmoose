#pragma once

#include "StanzaIdPayload.h"
#include "StanzaIdPayloadParser.h"
#include "StanzaIdPayloadParserFactory.h"
#include "StanzaIdPayloadSerializer.h"

#include <QObject>

#include <Swiften/Swiften.h>

// https://xmpp.org/extensions/xep-0359.html
// unique and stable IDs for messages, used to identify messages in xep-0313 (MAM)
class StanzaId : public QObject
{
    Q_OBJECT
public:
    StanzaId(QObject *parent = 0);
    void setupWithClient(Swift::Client *client);

signals:

public slots:

private:
    StanzaIdPayloadParserFactory stanzaIdPayloadParserFactory_;
    StanzaIdPayloadSerializer stanzaIdPayloadSerializer_;
};
