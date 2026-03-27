#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aspose::email::foss::msg::mime
{

struct mime_header
{
    std::string name;
    std::string value;
};

struct mime_entity
{
    std::vector<mime_header> headers;
    std::string content_type {"text/plain"};
    std::string charset {"utf-8"};
    std::string content_transfer_encoding;
    std::string content_disposition;
    std::string filename;
    std::string content_id;
    std::string boundary;
    std::vector<std::uint8_t> body;
    std::vector<mime_entity> children;

    [[nodiscard]] bool is_multipart() const noexcept;
};

struct mime_message_model
{
    std::vector<mime_header> headers;
    mime_entity root;
};

} // namespace aspose::email::foss::msg::mime
