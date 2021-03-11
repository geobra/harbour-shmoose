#include "EncryptionPayloadParser.h"

#include <iostream>

/*
  <encryption xmlns="urn:xmpp:eme:0" namespace="eu.siacs.conversations.axolotl" name="OMEMO"></encryption>
 */

void EncryptionPayloadParser::handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes)
{
    if (level_ == 0)
    {
        getPayloadInternal()->setNamespace(attributes.getAttribute("namespace"));
        getPayloadInternal()->setName(attributes.getAttribute("name"));
    }

    level_++;
}

void EncryptionPayloadParser::handleEndElement(const std::string& element, const std::string& /* ns */)
{
    level_--;
}

void EncryptionPayloadParser::handleCharacterData(const std::string& data)
{
    //std::cout << "############### enc ################### data: " << data << std::endl;
}
