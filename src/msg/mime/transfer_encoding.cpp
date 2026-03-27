#include "transfer_encoding.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace aspose::email::foss::msg::mime
{

namespace
{

constexpr char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

[[nodiscard]] std::uint8_t from_hex(const char value)
{
    if (value >= '0' && value <= '9')
    {
        return static_cast<std::uint8_t>(value - '0');
    }

    if (value >= 'A' && value <= 'F')
    {
        return static_cast<std::uint8_t>(value - 'A' + 10);
    }

    if (value >= 'a' && value <= 'f')
    {
        return static_cast<std::uint8_t>(value - 'a' + 10);
    }

    return 0U;
}

} // namespace

std::vector<std::uint8_t> transfer_encoding_decoder::decode(const std::string& encoding, const std::vector<std::uint8_t>& data)
{
    if (encoding == "base64")
    {
        std::vector<std::uint8_t> output;
        int bits_collected = 0;
        unsigned int accumulator = 0;
        for (const auto byte : data)
        {
            if (byte == '=')
            {
                break;
            }

            const auto pointer = std::find(std::begin(base64_alphabet), std::end(base64_alphabet) - 1, static_cast<char>(byte));
            if (pointer == std::end(base64_alphabet) - 1)
            {
                if (!std::isspace(byte))
                {
                    continue;
                }

                continue;
            }

            accumulator = (accumulator << 6U) | static_cast<unsigned int>(pointer - std::begin(base64_alphabet));
            bits_collected += 6;
            if (bits_collected >= 8)
            {
                bits_collected -= 8;
                output.push_back(static_cast<std::uint8_t>((accumulator >> bits_collected) & 0xFFU));
            }
        }

        return output;
    }

    if (encoding == "quoted-printable")
    {
        std::vector<std::uint8_t> output;
        for (std::size_t index = 0; index < data.size(); ++index)
        {
            if (data[index] == '=' && index + 2U < data.size())
            {
                if (data[index + 1U] == '\r' && data[index + 2U] == '\n')
                {
                    index += 2U;
                    continue;
                }

                output.push_back(static_cast<std::uint8_t>((from_hex(static_cast<char>(data[index + 1U])) << 4U) | from_hex(static_cast<char>(data[index + 2U]))));
                index += 2U;
                continue;
            }

            output.push_back(data[index]);
        }

        return output;
    }

    return data;
}

std::vector<std::uint8_t> transfer_encoding_encoder::encode_base64(const std::vector<std::uint8_t>& data)
{
    std::vector<std::uint8_t> output;
    for (std::size_t index = 0; index < data.size(); index += 3U)
    {
        const auto a = data[index];
        const auto b = index + 1U < data.size() ? data[index + 1U] : 0U;
        const auto c = index + 2U < data.size() ? data[index + 2U] : 0U;

        output.push_back(base64_alphabet[(a >> 2U) & 0x3FU]);
        output.push_back(base64_alphabet[((a & 0x03U) << 4U) | ((b >> 4U) & 0x0FU)]);
        output.push_back(index + 1U < data.size() ? base64_alphabet[((b & 0x0FU) << 2U) | ((c >> 6U) & 0x03U)] : '=');
        output.push_back(index + 2U < data.size() ? base64_alphabet[c & 0x3FU] : '=');
    }

    return output;
}

std::vector<std::uint8_t> transfer_encoding_encoder::encode_quoted_printable(const std::vector<std::uint8_t>& data)
{
    static constexpr char hex[] = "0123456789ABCDEF";
    std::vector<std::uint8_t> output;
    std::size_t line_length = 0U;
    for (const auto byte : data)
    {
        const auto plain = (byte >= 33U && byte <= 126U && byte != '=') || byte == ' ' || byte == '\t';
        if (!plain)
        {
            if (line_length + 3U > 72U)
            {
                output.insert(output.end(), {'=', '\r', '\n'});
                line_length = 0U;
            }

            output.push_back('=');
            output.push_back(static_cast<std::uint8_t>(hex[(byte >> 4U) & 0x0FU]));
            output.push_back(static_cast<std::uint8_t>(hex[byte & 0x0FU]));
            line_length += 3U;
            continue;
        }

        if (line_length + 1U > 72U)
        {
            output.insert(output.end(), {'=', '\r', '\n'});
            line_length = 0U;
        }

        output.push_back(byte);
        ++line_length;
    }

    return output;
}

} // namespace aspose::email::foss::msg::mime
