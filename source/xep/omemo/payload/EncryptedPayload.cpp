#include "EncryptedPayload.h"

const std::string& EncryptedPayload::getNamespace() const
{
    return namespace_;
}

void EncryptedPayload::setNamespace(const std::string& ns)
{
    namespace_ = ns;
}

const std::string& EncryptedPayload::getName() const
{
    return name_;
}

void EncryptedPayload::setName(const std::string& name)
{
    name_ = name;
}
