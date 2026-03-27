#include "aspose/email/foss/msg/msg_writer.hpp"

#include <fstream>

#include "aspose/email/foss/cfb/cfb_writer.hpp"
#include "aspose/email/foss/msg/msg_document.hpp"
#include "aspose/email/foss/msg/msg_exception.hpp"

namespace aspose::email::foss::msg
{

std::vector<std::uint8_t> msg_writer::to_bytes(const msg_document& document)
{
    return cfb::cfb_writer::to_bytes(document.to_cfb_document());
}

void msg_writer::write_file(const msg_document& document, const std::filesystem::path& path)
{
    std::ofstream stream(path, std::ios::binary);
    if (!stream)
    {
        throw msg_exception("Unable to open output path for writing: " + path.string());
    }

    write_stream(document, stream);
}

void msg_writer::write_stream(const msg_document& document, std::ostream& stream)
{
    const auto bytes = to_bytes(document);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!stream)
    {
        throw msg_exception("Failed to write MSG payload.");
    }
}

} // namespace aspose::email::foss::msg
