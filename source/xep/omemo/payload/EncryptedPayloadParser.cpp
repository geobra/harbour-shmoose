#include "EncryptedPayloadParser.h"

#include <iostream>

/*
  <encrypted xmlns="eu.siacs.conversations.axolotl">
   <header sid="44305397">
    <key rid="602241332">MwohBbriGvbMDPGJGdL1TVia2YflrwTyKeEmxGID0U/2OHtKEAEYASIwvtYh4jdstbmtvQHNywc0/Y/7zX1MMHtTkpB0naDCvyatlIVewRpex0yzu+9LUAnqmfayYOpfPEA=</key>
    <iv>MewBTeM3puSFFWAQ</iv>
   </header>
   <payload>H7LOYpp7aWOiAQhP9KbpMSALEQ==</payload>
  </encrypted>

  <encryption xmlns="urn:xmpp:eme:0" namespace="eu.siacs.conversations.axolotl" name="OMEMO"></encryption>

 */

void EncryptedPayloadParser::handleStartElement(const std::string& element, const std::string& ns, const Swift::AttributeMap& attributes)
{
    if (level_ == 0)
    {
        getPayloadInternal()->setNamespace(attributes.getAttribute("namespace"));
        getPayloadInternal()->setName(attributes.getAttribute("name"));
    }

    level_++;
}

void EncryptedPayloadParser::handleEndElement(const std::string& element, const std::string& /* ns */)
{
    level_--;
}

void EncryptedPayloadParser::handleCharacterData(const std::string& data)
{
    //std::cout << "############### enc ################### data: " << data << std::endl;
}
