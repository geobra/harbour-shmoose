#pragma once

#include <Swiften/Swiften.h>

class EncryptedPayload : public Swift::Payload
{
  public:
    EncryptedPayload() {}

    const std::string& getNamespace() const;
    void setNamespace(const std::string& ns);

    const std::string& getName() const;
    void setName(const std::string& name);

  private:
    std::string namespace_{};
    std::string name_{};
};
