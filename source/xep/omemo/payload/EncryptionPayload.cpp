#include "EncryptionPayload.h"

const std::string& EncryptionPayload::getNamespace() const
{
    return namespace_;
}

void EncryptionPayload::setNamespace(const std::string& ns)
{
    namespace_ = ns;
}

const std::string& EncryptionPayload::getName() const
{
    return name_;
}

void EncryptionPayload::setName(const std::string& name)
{
    name_ = name;
}
