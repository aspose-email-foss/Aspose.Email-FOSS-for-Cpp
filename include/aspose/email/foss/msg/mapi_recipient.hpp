#pragma once

#include <string>

#include "aspose/email/foss/msg/mapi_property_collection.hpp"

namespace aspose::email::foss::msg
{

class mapi_recipient
{
public:
    std::string display_name;
    std::string email_address;
    int recipient_type {1};
    std::string address_type {"SMTP"};
    mapi_property_collection properties;
};

} // namespace aspose::email::foss::msg
