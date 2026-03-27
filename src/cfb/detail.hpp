#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>

namespace aspose::email::foss::cfb::detail
{

[[nodiscard]] std::vector<std::uint8_t> read_all(std::istream& stream);
[[nodiscard]] std::string utf16le_to_utf8(const std::uint8_t* data, std::size_t size);
[[nodiscard]] std::vector<std::uint8_t> utf8_to_utf16le(const std::string& value);
[[nodiscard]] std::size_t utf16le_byte_count(const std::string& value);
[[nodiscard]] int compare_directory_entry_names(const std::string& left, const std::string& right);
[[nodiscard]] std::uint16_t read_u16(const std::vector<std::uint8_t>& data, std::size_t offset);
[[nodiscard]] std::uint32_t read_u32(const std::vector<std::uint8_t>& data, std::size_t offset);
[[nodiscard]] std::uint64_t read_u64(const std::vector<std::uint8_t>& data, std::size_t offset);
void write_u16(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint16_t value);
void write_u32(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint32_t value);
void write_u64(std::vector<std::uint8_t>& buffer, std::size_t offset, std::uint64_t value);

} // namespace aspose::email::foss::cfb::detail
