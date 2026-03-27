#include "aspose/email/foss/msg/mapi_attachment.hpp"

#include "../cfb/detail.hpp"

namespace aspose::email::foss::msg
{

mapi_attachment mapi_attachment::from_bytes(std::string filename, std::vector<std::uint8_t> data, std::string mime_type, std::string content_id)
{
    mapi_attachment attachment;
    attachment.filename = std::move(filename);
    attachment.data = std::move(data);
    attachment.mime_type = std::move(mime_type);
    attachment.content_id = std::move(content_id);
    return attachment;
}

mapi_attachment mapi_attachment::from_stream(std::string filename, std::istream& stream, std::string mime_type, std::string content_id)
{
    return from_bytes(std::move(filename), cfb::detail::read_all(stream), std::move(mime_type), std::move(content_id));
}

void mapi_attachment::load_data(std::istream& stream)
{
    data = cfb::detail::read_all(stream);
}

bool mapi_attachment::is_embedded_message() const noexcept
{
    return static_cast<bool>(embedded_message);
}

} // namespace aspose::email::foss::msg
