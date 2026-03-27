#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aspose::email::foss::msg::mime
{

class transfer_encoding_decoder
{
public:
    [[nodiscard]] static std::vector<std::uint8_t> decode(const std::string& encoding, const std::vector<std::uint8_t>& data);
};

class transfer_encoding_encoder
{
public:
    [[nodiscard]] static std::vector<std::uint8_t> encode_base64(const std::vector<std::uint8_t>& data);
    [[nodiscard]] static std::vector<std::uint8_t> encode_quoted_printable(const std::vector<std::uint8_t>& data);
};

} // namespace aspose::email::foss::msg::mime
