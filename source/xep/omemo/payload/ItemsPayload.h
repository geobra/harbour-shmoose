#pragma once

#include <Swiften/Swiften.h>

class ItemsPayload : public Swift::Payload
{
  public:
    ItemsPayload() {}

    const std::string& getItemsPayload() const;
    void setItemsPayload(const std::string& payload);

    const std::string& getNode() const;
    void setNode(const std::string& node);

  private:
    std::string itemsPayload_{};
    std::string node_{};
};
