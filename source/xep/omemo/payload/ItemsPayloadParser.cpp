#include "ItemsPayloadParser.h"

#include <iostream>

ItemsPayloadParser::ItemsPayloadParser()
{

}

void ItemsPayloadParser::handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes)
{
    if (level_ == 1 && element.compare("items") == 0)
    {
        collectStanzas = true;
        getPayloadInternal()->setNode(attributes.getAttribute("node"));
    }

    if (collectStanzas == true)
    {
        itemsStanza += "<" + element + " ";
        for(auto entry: attributes.getEntries())
        {
            itemsStanza += entry.getAttribute().getName() + "=\"" + entry.getValue() + "\" ";
        }
        itemsStanza += ">";
    }

    level_++;
}

void ItemsPayloadParser::handleEndElement(const std::string& element, const std::string& /* ns */)
{
    itemsStanza += "</" + element + ">";
    level_--;

    if (level_ == 1)
    {
        collectStanzas = false;
        //std::cout << "######### is: " << itemsStanza << std::endl;
        getPayloadInternal()->setItemsPayload(itemsStanza);
    }
}

void ItemsPayloadParser::handleCharacterData(const std::string& data)
{
    //std::cout << "################################## data: " << data << std::endl;
}
