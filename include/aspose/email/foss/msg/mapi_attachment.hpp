#pragma once

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "aspose/email/foss/msg/mapi_property_collection.hpp"

namespace aspose::email::foss::msg
{

class mapi_message;

class mapi_attachment
{
public:
    static mapi_attachment from_bytes(std::string filename, std::vector<std::uint8_t> data, std::string mime_type = {}, std::string content_id = {});
    static mapi_attachment from_stream(std::string filename, std::istream& stream, std::string mime_type = {}, std::string content_id = {});

    void load_data(std::istream& stream);

    [[nodiscard]] bool is_embedded_message() const noexcept;

    std::string filename;
    std::vector<std::uint8_t> data;
    std::string mime_type;
    std::string content_id;
    std::shared_ptr<mapi_message> embedded_message;
    mapi_property_collection properties;
};

} // namespace aspose::email::foss::msg
