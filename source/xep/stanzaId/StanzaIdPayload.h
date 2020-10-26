#pragma once

#include <Swiften/Swiften.h>

// Swiften payload extension for xep-0359
// See https://swift.im/swiften/guide/#Section-Extending
class StanzaIdPayload : public Swift::Payload
{
public:
    StanzaIdPayload();

    const std::string& getBy() const;
    const std::string& getId() const;
    void setBy(const std::string& by);
    void setId(const std::string& id);

private:
    std::string by_{};
    std::string id_{};
};
