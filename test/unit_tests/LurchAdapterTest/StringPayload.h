#pragma once

#include <string>
#include <Swiften/Swiften.h>

class StringPayload : public Swift::Payload
{
public:
    StringPayload(const std::string& s = "") : text_(s) {}

private:
    std::string text_;
};

