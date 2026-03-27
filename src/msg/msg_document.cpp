#include "aspose/email/foss/msg/msg_document.hpp"

#include <regex>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/cfb/cfb_storage.hpp"
#include "aspose/email/foss/cfb/cfb_stream.hpp"
#include "aspose/email/foss/msg/common_message_property_id.hpp"
#include "aspose/email/foss/msg/msg_constants.hpp"
#include "aspose/email/foss/msg/property_type_code.hpp"
#include "../cfb/detail.hpp"

namespace aspose::email::foss::msg
{

namespace
{

const std::regex recipient_name_pattern("^__recip_version1\\.0_#[0-9A-Fa-f]{8}$");
const std::regex attachment_name_pattern("^__attach_version1\\.0_#[0-9A-Fa-f]{8}$");

[[nodiscard]] std::uint32_t read_attach_method(const msg_storage& storage)
{
    const auto* property_stream = storage.find_stream(std::string(msg_constants::property_stream_name));
    if (property_stream == nullptr || property_stream->data.size() < 24U)
    {
        return 0U;
    }

    const auto header_size = storage.role == msg_storage_role::message || storage.role == msg_storage_role::embedded_message ? 32U : 8U;
    for (std::size_t offset = header_size; offset + 16U <= property_stream->data.size(); offset += 16U)
    {
        const auto property_tag = cfb::detail::read_u32(property_stream->data, offset);
        if (property_tag == ((static_cast<std::uint32_t>(common_message_property_id::attach_method) << 16U) | static_cast<std::uint16_t>(property_type_code::ptyp_integer32)))
        {
            return cfb::detail::read_u32(property_stream->data, offset + 8U);
        }
    }

    return 0U;
}

[[nodiscard]] msg_storage_role classify_storage_role(
    const cfb::cfb_reader& cfb_reader,
    const msg_storage_role parent_role,
    const cfb::directory_entry& child,
    const std::uint32_t attach_method)
{
    if (parent_role == msg_storage_role::message || parent_role == msg_storage_role::embedded_message)
    {
        if (child.name == msg_constants::named_property_mapping_storage_name)
        {
            return msg_storage_role::named_property_mapping;
        }

        if (std::regex_match(child.name, recipient_name_pattern))
        {
            return msg_storage_role::recipient;
        }

        if (std::regex_match(child.name, attachment_name_pattern))
        {
            return msg_storage_role::attachment;
        }

        return msg_storage_role::generic;
    }

    if (parent_role == msg_storage_role::attachment && child.name == msg_constants::embedded_message_storage_name)
    {
        if (attach_method == 5U)
        {
            return msg_storage_role::embedded_message;
        }

        if (attach_method == 6U)
        {
            return msg_storage_role::custom_attachment;
        }

        return cfb_reader.find_child_by_name(child.stream_id, std::string(msg_constants::property_stream_name)).has_value()
            ? msg_storage_role::embedded_message
            : msg_storage_role::custom_attachment;
    }

    return msg_storage_role::generic;
}

} // namespace

msg_document::msg_document(
    msg_storage root,
    const std::uint16_t major_version,
    const std::uint16_t minor_version,
    const std::uint32_t transaction_signature_number,
    const bool strict)
    : root_(std::move(root)),
      major_version_(major_version),
      minor_version_(minor_version),
      transaction_signature_number_(transaction_signature_number),
      strict_(strict)
{
}

msg_storage& msg_document::root() noexcept
{
    return root_;
}

const msg_storage& msg_document::root() const noexcept
{
    return root_;
}

std::uint16_t msg_document::major_version() const noexcept
{
    return major_version_;
}

std::uint16_t msg_document::minor_version() const noexcept
{
    return minor_version_;
}

std::uint32_t msg_document::transaction_signature_number() const noexcept
{
    return transaction_signature_number_;
}

bool msg_document::strict() const noexcept
{
    return strict_;
}

msg_document msg_document::from_reader(const msg_reader& reader)
{
    const auto build_storage = [&reader](const auto& self, const std::uint32_t stream_id, const msg_storage_role role) -> msg_storage
    {
        const auto& cfb_reader = reader.cfb();
        const auto& entry = cfb_reader.get_entry(stream_id);
        msg_storage storage(entry.name, role);
        storage.clsid = entry.clsid;
        storage.state_bits = entry.state_bits;
        storage.creation_time = entry.creation_time;
        storage.modified_time = entry.modified_time;

        for (const auto child_id : cfb_reader.child_ids(stream_id))
        {
            const auto& child = cfb_reader.get_entry(child_id);
            if (child.is_stream())
            {
                msg_stream stream(child.name, cfb_reader.get_stream_data(child.stream_id));
                stream.clsid = child.clsid;
                stream.state_bits = child.state_bits;
                stream.creation_time = child.creation_time;
                stream.modified_time = child.modified_time;
                storage.add_stream(std::move(stream));
                continue;
            }

            const auto attach_method = role == msg_storage_role::attachment ? read_attach_method(storage) : 0U;
            const auto child_role = classify_storage_role(cfb_reader, role, child, attach_method);
            storage.add_storage(self(self, child.stream_id, child_role));
        }

        return storage;
    };

    const auto& cfb_reader = reader.cfb();
    return msg_document(
        build_storage(build_storage, cfb::cfb_constants::root_stream_id, msg_storage_role::message),
        cfb_reader.header().major_version,
        cfb_reader.header().minor_version,
        cfb_reader.header().transaction_signature_number,
        reader.strict());
}

msg_document msg_document::from_file(const std::filesystem::path& path, const bool strict)
{
    return from_reader(msg_reader::from_file(path, strict));
}

msg_document msg_document::from_stream(std::istream& stream, const bool strict)
{
    return from_reader(msg_reader::from_stream(stream, strict));
}

cfb::cfb_document msg_document::to_cfb_document() const
{
    const auto to_cfb_storage = [](const auto& self, const msg_storage& storage) -> cfb::cfb_storage
    {
        cfb::cfb_storage result(storage.name);
        result.clsid() = storage.clsid;
        result.set_state_bits(storage.state_bits);
        result.set_creation_time(storage.creation_time);
        result.set_modified_time(storage.modified_time);

        for (const auto& stream : storage.streams)
        {
            cfb::cfb_stream cfb_stream(stream.name, stream.data);
            cfb_stream.clsid() = stream.clsid;
            cfb_stream.set_state_bits(stream.state_bits);
            cfb_stream.set_creation_time(stream.creation_time);
            cfb_stream.set_modified_time(stream.modified_time);
            result.add_stream(std::move(cfb_stream));
        }

        for (const auto& child : storage.storages)
        {
            result.add_storage(self(self, child));
        }

        return result;
    };

    return cfb::cfb_document(to_cfb_storage(to_cfb_storage, root_), major_version_, minor_version_, transaction_signature_number_);
}

} // namespace aspose::email::foss::msg
