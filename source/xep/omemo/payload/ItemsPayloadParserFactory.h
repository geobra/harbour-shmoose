#pragma once

#include "ItemsPayloadParser.h"

class ItemsPayloadParserFactory : public Swift::GenericPayloadParserFactory<ItemsPayloadParser>
{
public:
    ItemsPayloadParserFactory();
};
