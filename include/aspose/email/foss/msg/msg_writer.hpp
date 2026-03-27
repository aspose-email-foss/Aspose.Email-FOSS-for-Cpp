#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <vector>

namespace aspose::email::foss::msg
{

class msg_document;

class msg_writer
{
public:
    [[nodiscard]] static std::vector<std::uint8_t> to_bytes(const msg_document& document);
    static void write_file(const msg_document& document, const std::filesystem::path& path);
    static void write_stream(const msg_document& document, std::ostream& stream);
};

} // namespace aspose::email::foss::msg
