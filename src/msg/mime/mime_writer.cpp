#include "mime_writer.hpp"

#include <array>

#include "../../cfb/detail.hpp"
#include "transfer_encoding.hpp"

namespace aspose::email::foss::msg::mime
{

std::string mime_boundary_factory::create()
{
    return "----=_AsposeEmailFossBoundary_0001";
}

std::vector<std::uint8_t> mime_writer::write(const mime_message_model& model) const
{
    std::vector<std::uint8_t> output;
    for (const auto& header : model.headers)
    {
        output.insert(output.end(), header.name.begin(), header.name.end());
        output.insert(output.end(), {':', ' '});
        output.insert(output.end(), header.value.begin(), header.value.end());
        output.insert(output.end(), {'\r', '\n'});
    }

    output.insert(output.end(), {'\r', '\n'});
    const auto body = write_entity(model.root);
    output.insert(output.end(), body.begin(), body.end());
    return output;
}

std::vector<std::uint8_t> mime_writer::write_entity(const mime_entity& entity) const
{
    std::vector<std::uint8_t> output;
    if (entity.is_multipart())
    {
        const auto boundary = entity.boundary.empty() ? mime_boundary_factory::create() : entity.boundary;
        for (const auto& child : entity.children)
        {
            output.insert(output.end(), {'-', '-'});
            output.insert(output.end(), boundary.begin(), boundary.end());
            output.insert(output.end(), {'\r', '\n'});
            for (const auto& header : child.headers)
            {
                output.insert(output.end(), header.name.begin(), header.name.end());
                output.insert(output.end(), {':', ' '});
                output.insert(output.end(), header.value.begin(), header.value.end());
                output.insert(output.end(), {'\r', '\n'});
            }

            output.insert(output.end(), {'\r', '\n'});
            const auto child_payload = write_entity(child);
            output.insert(output.end(), child_payload.begin(), child_payload.end());
            output.insert(output.end(), {'\r', '\n'});
        }

        output.insert(output.end(), {'-', '-'});
        output.insert(output.end(), boundary.begin(), boundary.end());
        output.insert(output.end(), {'-', '-', '\r', '\n'});
        return output;
    }

    if (entity.content_transfer_encoding == "base64")
    {
        const auto encoded = transfer_encoding_encoder::encode_base64(entity.body);
        output.insert(output.end(), encoded.begin(), encoded.end());
        return output;
    }

    if (entity.content_transfer_encoding == "quoted-printable")
    {
        const auto encoded = transfer_encoding_encoder::encode_quoted_printable(entity.body);
        output.insert(output.end(), encoded.begin(), encoded.end());
        return output;
    }

    output.insert(output.end(), entity.body.begin(), entity.body.end());
    return output;
}

} // namespace aspose::email::foss::msg::mime
