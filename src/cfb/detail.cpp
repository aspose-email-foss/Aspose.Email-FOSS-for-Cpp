#include "detail.hpp"

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <iterator>
#include <locale>
#include <stdexcept>
#include <vector>

namespace aspose::email::foss::cfb::detail
{

namespace
{

[[nodiscard]] char16_t ascii_upper(char16_t value) noexcept
{
    return value >= u'a' && value <= u'z' ? static_cast<char16_t>(value - 32) : value;
}

[[nodiscard]] std::u16string utf8_to_utf16(const std::string& value)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.from_bytes(value);
}

} // namespace

std::vector<std::uint8_t> read_all(std::istream& stream)
{
    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>());
}

std::string utf16le_to_utf8(const std::uint8_t* data, const std::size_t size)
{
    if ((size % 2) != 0U)
    {
        throw std::runtime_error("UTF-16LE byte sequence length must be even.");
    }

    std::u16string value(size / 2, u'\0');
    for (std::size_t index = 0; index < value.size(); ++index)
    {
        const auto low = static_cast<std::uint16_t>(data[index * 2]);
        const auto high = static_cast<std::uint16_t>(data[(index * 2) + 1]) << 8U;
        value[index] = static_cast<char16_t>(low | high);
    }

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.to_bytes(value);
}

std::vector<std::uint8_t> utf8_to_utf16le(const std::string& value)
{
    const auto utf16 = utf8_to_utf16(value);
    std::vector<std::uint8_t> bytes;
    bytes.reserve(utf16.size() * 2U);
    for (const auto code_unit : utf16)
    {
        bytes.push_back(static_cast<std::uint8_t>(code_unit & 0x00FFU));
        bytes.push_back(static_cast<std::uint8_t>((static_cast<std::uint16_t>(code_unit) >> 8U) & 0x00FFU));
    }

    return bytes;
}

std::size_t utf16le_byte_count(const std::string& value)
{
    return utf8_to_utf16le(value).size();
}

int compare_directory_entry_names(const std::string& left, const std::string& right)
{
    const auto left_utf16 = utf8_to_utf16(left);
    const auto right_utf16 = utf8_to_utf16(right);
    const auto left_length = (left_utf16.size() * 2U) + 2U;
    const auto right_length = (right_utf16.size() * 2U) + 2U;
    if (left_length != right_length)
    {
        return left_length < right_length ? -1 : 1;
    }

    const auto pair_count = std::min(left_utf16.size(), right_utf16.size());
    for (std::size_t index = 0; index < pair_count; ++index)
    {
        const auto left_value = ascii_upper(left_utf16[index]);
        const auto right_value = ascii_upper(right_utf16[index]);
        if (left_value != right_value)
        {
            return left_value < right_value ? -1 : 1;
        }
    }

    return 0;
}

std::uint16_t read_u16(const std::vector<std::uint8_t>& data, const std::size_t offset)
{
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(data.at(offset)) |
        (static_cast<std::uint16_t>(data.at(offset + 1U)) << 8U));
}

std::uint32_t read_u32(const std::vector<std::uint8_t>& data, const std::size_t offset)
{
    return static_cast<std::uint32_t>(data.at(offset)) |
        (static_cast<std::uint32_t>(data.at(offset + 1U)) << 8U) |
        (static_cast<std::uint32_t>(data.at(offset + 2U)) << 16U) |
        (static_cast<std::uint32_t>(data.at(offset + 3U)) << 24U);
}

std::uint64_t read_u64(const std::vector<std::uint8_t>& data, const std::size_t offset)
{
    return static_cast<std::uint64_t>(read_u32(data, offset)) |
        (static_cast<std::uint64_t>(read_u32(data, offset + 4U)) << 32U);
}

void write_u16(std::vector<std::uint8_t>& buffer, const std::size_t offset, const std::uint16_t value)
{
    buffer.at(offset) = static_cast<std::uint8_t>(value & 0x00FFU);
    buffer.at(offset + 1U) = static_cast<std::uint8_t>((value >> 8U) & 0x00FFU);
}

void write_u32(std::vector<std::uint8_t>& buffer, const std::size_t offset, const std::uint32_t value)
{
    buffer.at(offset) = static_cast<std::uint8_t>(value & 0x000000FFU);
    buffer.at(offset + 1U) = static_cast<std::uint8_t>((value >> 8U) & 0x000000FFU);
    buffer.at(offset + 2U) = static_cast<std::uint8_t>((value >> 16U) & 0x000000FFU);
    buffer.at(offset + 3U) = static_cast<std::uint8_t>((value >> 24U) & 0x000000FFU);
}

void write_u64(std::vector<std::uint8_t>& buffer, const std::size_t offset, const std::uint64_t value)
{
    write_u32(buffer, offset, static_cast<std::uint32_t>(value & 0xFFFFFFFFULL));
    write_u32(buffer, offset + 4U, static_cast<std::uint32_t>((value >> 32U) & 0xFFFFFFFFULL));
}

} // namespace aspose::email::foss::cfb::detail
