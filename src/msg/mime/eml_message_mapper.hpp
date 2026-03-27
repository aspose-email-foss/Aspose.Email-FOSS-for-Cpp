#pragma once

#include "mime_model.hpp"

namespace aspose::email::foss::msg
{

class mapi_message;

namespace mime
{

class eml_message_mapper
{
public:
    [[nodiscard]] static mapi_message to_mapi_message(const mime_message_model& model);
    [[nodiscard]] static mime_message_model to_mime_message(const mapi_message& message);
};

} // namespace mime

} // namespace aspose::email::foss::msg
