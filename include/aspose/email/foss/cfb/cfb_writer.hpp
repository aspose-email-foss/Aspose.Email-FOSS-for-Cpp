#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <vector>

namespace aspose::email::foss::cfb
{

class cfb_document;

class cfb_writer
{
public:
    [[nodiscard]] static std::vector<std::uint8_t> to_bytes(const cfb_document& document);
    static void write_file(const cfb_document& document, const std::filesystem::path& path);
    static void write_stream(const cfb_document& document, std::ostream& stream);
};

} // namespace aspose::email::foss::cfb
