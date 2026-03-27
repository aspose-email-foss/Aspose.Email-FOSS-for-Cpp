#include "aspose/email/foss/msg/mapi_message.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <regex>
#include <type_traits>
#include <utility>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/msg/common_message_property_id.hpp"
#include "aspose/email/foss/msg/msg_exception.hpp"
#include "aspose/email/foss/msg/msg_writer.hpp"
#include "aspose/email/foss/msg/property_type_code.hpp"
#include "mime/eml_message_mapper.hpp"
#include "mime/mime_reader.hpp"
#include "mime/mime_writer.hpp"
#include "../cfb/detail.hpp"

namespace aspose::email::foss::msg
{

namespace
{

const std::regex direct_stream_pattern("^__substg1\\.0_([0-9A-Fa-f]{4})([0-9A-Fa-f]{4})$");

template <typename Enum>
[[nodiscard]] constexpr auto to_underlying(const Enum value) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(value);
}

constexpr std::uint16_t recipient_type_property_id = 0x0C15;
constexpr std::uint16_t display_name_property_id = 0x3001;
constexpr std::uint16_t address_type_property_id = 0x3002;
constexpr std::uint16_t email_address_property_id = 0x3003;
constexpr std::uint16_t attach_long_pathname_property_id = 0x370D;
constexpr std::uint16_t attach_extension_property_id = 0x3703;

[[nodiscard]] bool is_fixed_inline_type(const std::uint16_t property_type) noexcept
{
    switch (property_type)
    {
    case to_underlying(property_type_code::ptyp_integer16):
    case to_underlying(property_type_code::ptyp_integer32):
    case to_underlying(property_type_code::ptyp_floating32):
    case to_underlying(property_type_code::ptyp_floating64):
    case to_underlying(property_type_code::ptyp_currency):
    case to_underlying(property_type_code::ptyp_floating_time):
    case to_underlying(property_type_code::ptyp_error_code):
    case to_underlying(property_type_code::ptyp_boolean):
    case to_underlying(property_type_code::ptyp_integer64):
    case to_underlying(property_type_code::ptyp_time):
        return true;
    default:
        return false;
    }
}

[[nodiscard]] std::string trim_zero_terminated_string(std::string value)
{
    while (!value.empty() && value.back() == '\0')
    {
        value.pop_back();
    }

    return value;
}

[[nodiscard]] std::string decode_unicode_string(std::vector<std::uint8_t> data)
{
    while (data.size() >= 2U && data[data.size() - 1U] == 0U && data[data.size() - 2U] == 0U)
    {
        data.resize(data.size() - 2U);
    }

    return cfb::detail::utf16le_to_utf8(data.data(), data.size());
}

[[nodiscard]] std::string decode_string8(std::vector<std::uint8_t> data)
{
    while (!data.empty() && data.back() == 0U)
    {
        data.pop_back();
    }

    return std::string(data.begin(), data.end());
}

[[nodiscard]] std::string any_to_string(const std::any& value)
{
    if (const auto* text = std::any_cast<std::string>(&value))
    {
        return *text;
    }

    return {};
}

[[nodiscard]] std::vector<std::uint8_t> any_to_binary(const std::any& value)
{
    if (const auto* data = std::any_cast<std::vector<std::uint8_t>>(&value))
    {
        return *data;
    }

    return {};
}

[[nodiscard]] std::int32_t any_to_int32(const std::any& value)
{
    if (const auto* item = std::any_cast<std::int32_t>(&value))
    {
        return *item;
    }

    if (const auto* item = std::any_cast<std::uint32_t>(&value))
    {
        return static_cast<std::int32_t>(*item);
    }

    if (const auto* item = std::any_cast<int>(&value))
    {
        return *item;
    }

    return 0;
}

[[nodiscard]] std::uint64_t any_to_uint64(const std::any& value)
{
    if (const auto* item = std::any_cast<std::uint64_t>(&value))
    {
        return *item;
    }

    if (const auto* item = std::any_cast<std::int64_t>(&value))
    {
        return static_cast<std::uint64_t>(*item);
    }

    return 0U;
}

[[nodiscard]] bool any_to_bool(const std::any& value)
{
    if (const auto* item = std::any_cast<bool>(&value))
    {
        return *item;
    }

    return false;
}

struct parsed_property_bag
{
    mapi_property_collection properties;
};

[[nodiscard]] parsed_property_bag parse_property_bag(const msg_storage& storage, const bool unicode_strings)
{
    parsed_property_bag result;
    const auto* property_stream = storage.find_stream(std::string(msg_constants::property_stream_name));
    if (property_stream == nullptr)
    {
        return result;
    }

    std::map<std::pair<std::uint16_t, std::uint16_t>, std::vector<std::uint8_t>> direct_streams;
    for (const auto& stream : storage.streams)
    {
        std::smatch match;
        if (std::regex_match(stream.name, match, direct_stream_pattern))
        {
            direct_streams[std::make_pair(
                static_cast<std::uint16_t>(std::stoul(match[1].str(), nullptr, 16)),
                static_cast<std::uint16_t>(std::stoul(match[2].str(), nullptr, 16)))] = stream.data;
        }
    }

    const auto header_size = storage.role == msg_storage_role::message || storage.role == msg_storage_role::embedded_message ? 32U : 8U;
    for (std::size_t offset = header_size; offset + 16U <= property_stream->data.size(); offset += 16U)
    {
        const auto property_tag = cfb::detail::read_u32(property_stream->data, offset);
        const auto flags = cfb::detail::read_u32(property_stream->data, offset + 4U);
        const auto property_id = static_cast<std::uint16_t>(property_tag >> 16U);
        const auto property_type = static_cast<std::uint16_t>(property_tag & 0xFFFFU);
        std::any value;
        if (is_fixed_inline_type(property_type))
        {
            switch (property_type)
            {
            case to_underlying(property_type_code::ptyp_integer16):
                value = static_cast<std::int16_t>(cfb::detail::read_u16(property_stream->data, offset + 8U));
                break;
            case to_underlying(property_type_code::ptyp_integer32):
                value = static_cast<std::int32_t>(cfb::detail::read_u32(property_stream->data, offset + 8U));
                break;
            case to_underlying(property_type_code::ptyp_boolean):
                value = cfb::detail::read_u16(property_stream->data, offset + 8U) != 0U;
                break;
            case to_underlying(property_type_code::ptyp_integer64):
            case to_underlying(property_type_code::ptyp_currency):
            case to_underlying(property_type_code::ptyp_time):
                value = cfb::detail::read_u64(property_stream->data, offset + 8U);
                break;
            default:
                value = static_cast<std::int32_t>(cfb::detail::read_u32(property_stream->data, offset + 8U));
                break;
            }
        }
        else
        {
            const auto iterator = direct_streams.find(std::make_pair(property_id, property_type));
            const auto payload = iterator == direct_streams.end() ? std::vector<std::uint8_t> {} : iterator->second;
            switch (property_type)
            {
            case to_underlying(property_type_code::ptyp_string):
                value = decode_unicode_string(payload);
                break;
            case to_underlying(property_type_code::ptyp_string8):
                value = decode_string8(payload);
                break;
            case to_underlying(property_type_code::ptyp_binary):
                value = payload;
                break;
            default:
                value = payload;
                break;
            }
        }

        result.properties.add(property_id, property_type, std::move(value), flags);
    }

    return result;
}

[[nodiscard]] std::vector<std::uint8_t> encode_unicode_string(const std::string& value)
{
    return cfb::detail::utf8_to_utf16le(value);
}

[[nodiscard]] std::vector<std::uint8_t> encode_string8(const std::string& value)
{
    return std::vector<std::uint8_t>(value.begin(), value.end());
}

[[nodiscard]] std::vector<std::uint8_t> encode_inline_value(const std::uint16_t property_type, const std::any& value)
{
    std::vector<std::uint8_t> result(8U, 0U);
    switch (property_type)
    {
    case to_underlying(property_type_code::ptyp_integer16):
        cfb::detail::write_u16(result, 0U, static_cast<std::uint16_t>(any_to_int32(value)));
        break;
    case to_underlying(property_type_code::ptyp_integer32):
        cfb::detail::write_u32(result, 0U, static_cast<std::uint32_t>(any_to_int32(value)));
        break;
    case to_underlying(property_type_code::ptyp_boolean):
        cfb::detail::write_u16(result, 0U, any_to_bool(value) ? 1U : 0U);
        break;
    case to_underlying(property_type_code::ptyp_integer64):
    case to_underlying(property_type_code::ptyp_currency):
    case to_underlying(property_type_code::ptyp_time):
        cfb::detail::write_u64(result, 0U, any_to_uint64(value));
        break;
    default:
        break;
    }

    return result;
}

struct serialized_property_bag
{
    msg_stream property_stream {std::string(msg_constants::property_stream_name)};
    std::vector<msg_stream> value_streams;
};

[[nodiscard]] std::string value_stream_name(const std::uint16_t property_id, const std::uint16_t property_type)
{
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "__substg1.0_%04X%04X", property_id, property_type);
    return std::string(buffer);
}

[[nodiscard]] serialized_property_bag serialize_property_bag(
    const mapi_property_collection& properties,
    const msg_storage_role role,
    const std::size_t recipient_count,
    const std::size_t attachment_count)
{
    serialized_property_bag result;
    auto& property_data = result.property_stream.data;
    if (role == msg_storage_role::message || role == msg_storage_role::embedded_message)
    {
        property_data.resize(32U, 0U);
        cfb::detail::write_u32(property_data, 8U, recipient_count == 0U ? 0U : static_cast<std::uint32_t>(recipient_count));
        cfb::detail::write_u32(property_data, 12U, attachment_count == 0U ? 0U : static_cast<std::uint32_t>(attachment_count));
        cfb::detail::write_u32(property_data, 16U, static_cast<std::uint32_t>(recipient_count));
        cfb::detail::write_u32(property_data, 20U, static_cast<std::uint32_t>(attachment_count));
    }
    else
    {
        property_data.resize(8U, 0U);
    }

    for (const auto& [key, property] : properties.items())
    {
        const auto property_tag = property.property_tag();
        const auto base_offset = property_data.size();
        property_data.resize(property_data.size() + 16U, 0U);
        cfb::detail::write_u32(property_data, base_offset, property_tag);
        cfb::detail::write_u32(property_data, base_offset + 4U, property.flags());
        if (is_fixed_inline_type(property.property_type()))
        {
            const auto inline_value = encode_inline_value(property.property_type(), property.value());
            std::copy(inline_value.begin(), inline_value.end(), property_data.begin() + static_cast<std::ptrdiff_t>(base_offset + 8U));
            continue;
        }

        if (property.property_type() == to_underlying(property_type_code::ptyp_object))
        {
            cfb::detail::write_u32(property_data, base_offset + 8U, 0xFFFFFFFFU);
            continue;
        }

        std::vector<std::uint8_t> payload;
        switch (property.property_type())
        {
        case to_underlying(property_type_code::ptyp_string):
            payload = encode_unicode_string(any_to_string(property.value()));
            cfb::detail::write_u32(property_data, base_offset + 8U, static_cast<std::uint32_t>(payload.size() + 2U));
            break;
        case to_underlying(property_type_code::ptyp_string8):
            payload = encode_string8(any_to_string(property.value()));
            cfb::detail::write_u32(property_data, base_offset + 8U, static_cast<std::uint32_t>(payload.size() + 1U));
            break;
        case to_underlying(property_type_code::ptyp_binary):
            payload = any_to_binary(property.value());
            cfb::detail::write_u32(property_data, base_offset + 8U, static_cast<std::uint32_t>(payload.size()));
            break;
        default:
            payload = any_to_binary(property.value());
            cfb::detail::write_u32(property_data, base_offset + 8U, static_cast<std::uint32_t>(payload.size()));
            break;
        }

        result.value_streams.emplace_back(value_stream_name(property.property_id(), property.property_type()), std::move(payload));
    }

    return result;
}

[[nodiscard]] std::string short_filename(const std::string& filename)
{
    if (filename.size() <= 8U)
    {
        return filename;
    }

    return filename.substr(0U, 8U);
}

} // namespace

mapi_message mapi_message::create(std::string subject, std::string body, const bool unicode_strings)
{
    mapi_message message;
    message.unicode_strings_ = unicode_strings;
    message.subject_ = std::move(subject);
    message.body_ = std::move(body);
    return message;
}

mapi_message mapi_message::from_file(const std::filesystem::path& path, const bool strict)
{
    auto reader = msg_reader::from_file(path, strict);
    auto message = from_msg_document(msg_document::from_reader(reader));
    message.validation_issues_ = reader.validation_issues();
    return message;
}

mapi_message mapi_message::from_stream(std::istream& stream, const bool strict)
{
    auto reader = msg_reader::from_stream(stream, strict);
    auto message = from_msg_document(msg_document::from_reader(reader));
    message.validation_issues_ = reader.validation_issues();
    return message;
}

mapi_message mapi_message::from_msg_document(const msg_document& document)
{
    mapi_message message;
    const auto parsed = parse_property_bag(document.root(), true);
    message.properties_ = parsed.properties;

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::subject)); property != nullptr)
    {
        message.subject_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::body)); property != nullptr)
    {
        message.body_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::body_html)); property != nullptr)
    {
        message.html_body_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::message_class)); property != nullptr)
    {
        message.message_class_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::sender_name)); property != nullptr)
    {
        message.sender_name_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::sender_email_address)); property != nullptr)
    {
        message.sender_email_address_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::sender_address_type)); property != nullptr)
    {
        message.sender_address_type_ = any_to_string(property->value());
    }

    if (const auto* property = message.properties_.get(to_underlying(common_message_property_id::internet_message_id)); property != nullptr)
    {
        message.internet_message_id_ = any_to_string(property->value());
    }

    for (const auto& storage : document.root().storages)
    {
        if (storage.role == msg_storage_role::recipient)
        {
            const auto recipient_props = parse_property_bag(storage, true).properties;
            mapi_recipient recipient;
            if (const auto* property = recipient_props.get(display_name_property_id); property != nullptr)
            {
                recipient.display_name = any_to_string(property->value());
            }

            if (const auto* property = recipient_props.get(email_address_property_id); property != nullptr)
            {
                recipient.email_address = any_to_string(property->value());
            }

            if (const auto* property = recipient_props.get(address_type_property_id); property != nullptr)
            {
                recipient.address_type = any_to_string(property->value());
            }

            if (const auto* property = recipient_props.get(recipient_type_property_id); property != nullptr)
            {
                recipient.recipient_type = any_to_int32(property->value());
            }

            recipient.properties = recipient_props;
            message.recipients_.push_back(std::move(recipient));
            continue;
        }

        if (storage.role == msg_storage_role::attachment)
        {
            const auto attachment_props = parse_property_bag(storage, true).properties;
            mapi_attachment attachment;
            if (const auto* property = attachment_props.get(to_underlying(common_message_property_id::attach_long_filename)); property != nullptr)
            {
                attachment.filename = any_to_string(property->value());
            }
            else if (const auto* property = attachment_props.get(to_underlying(common_message_property_id::attach_filename)); property != nullptr)
            {
                attachment.filename = any_to_string(property->value());
            }

            if (const auto* property = attachment_props.get(to_underlying(common_message_property_id::attach_mime_tag)); property != nullptr)
            {
                attachment.mime_type = any_to_string(property->value());
            }

            if (const auto* property = attachment_props.get(to_underlying(common_message_property_id::attach_content_id)); property != nullptr)
            {
                attachment.content_id = any_to_string(property->value());
            }

            if (const auto* property = attachment_props.get(to_underlying(common_message_property_id::attach_data_binary)); property != nullptr)
            {
                attachment.data = any_to_binary(property->value());
            }

            if (const auto* embedded = storage.find_storage(std::string(msg_constants::embedded_message_storage_name)); embedded != nullptr)
            {
                attachment.embedded_message = std::make_shared<mapi_message>(from_msg_document(msg_document(*embedded, document.major_version(), document.minor_version(), document.transaction_signature_number(), document.strict())));
            }

            attachment.properties = attachment_props;
            message.attachments_.push_back(std::move(attachment));
        }
    }

    return message;
}

mapi_message mapi_message::load_from_eml(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream)
    {
        throw msg_exception("Unable to open EML file: " + path.string());
    }

    return load_from_eml(stream);
}

mapi_message mapi_message::load_from_eml(std::istream& stream)
{
    mime::mime_reader reader;
    return mime::eml_message_mapper::to_mapi_message(reader.read(stream));
}

bool mapi_message::unicode_strings() const noexcept
{
    return unicode_strings_;
}

void mapi_message::set_unicode_strings(const bool value) noexcept
{
    unicode_strings_ = value;
}

const std::vector<std::string>& mapi_message::validation_issues() const noexcept
{
    return validation_issues_;
}

const std::string& mapi_message::subject() const noexcept
{
    return subject_;
}

void mapi_message::set_subject(std::string value)
{
    subject_ = std::move(value);
}

const std::string& mapi_message::body() const noexcept
{
    return body_;
}

void mapi_message::set_body(std::string value)
{
    body_ = std::move(value);
}

const std::string& mapi_message::html_body() const noexcept
{
    return html_body_;
}

void mapi_message::set_html_body(std::string value)
{
    html_body_ = std::move(value);
}

const std::string& mapi_message::message_class() const noexcept
{
    return message_class_;
}

void mapi_message::set_message_class(std::string value)
{
    message_class_ = std::move(value);
}

const std::string& mapi_message::sender_name() const noexcept
{
    return sender_name_;
}

void mapi_message::set_sender_name(std::string value)
{
    sender_name_ = std::move(value);
}

const std::string& mapi_message::sender_email_address() const noexcept
{
    return sender_email_address_;
}

void mapi_message::set_sender_email_address(std::string value)
{
    sender_email_address_ = std::move(value);
}

const std::string& mapi_message::sender_address_type() const noexcept
{
    return sender_address_type_;
}

void mapi_message::set_sender_address_type(std::string value)
{
    sender_address_type_ = std::move(value);
}

const std::string& mapi_message::internet_message_id() const noexcept
{
    return internet_message_id_;
}

void mapi_message::set_internet_message_id(std::string value)
{
    internet_message_id_ = std::move(value);
}

mapi_property_collection& mapi_message::properties() noexcept
{
    return properties_;
}

const mapi_property_collection& mapi_message::properties() const noexcept
{
    return properties_;
}

std::vector<mapi_recipient>& mapi_message::recipients() noexcept
{
    return recipients_;
}

const std::vector<mapi_recipient>& mapi_message::recipients() const noexcept
{
    return recipients_;
}

std::vector<mapi_attachment>& mapi_message::attachments() noexcept
{
    return attachments_;
}

const std::vector<mapi_attachment>& mapi_message::attachments() const noexcept
{
    return attachments_;
}

mapi_recipient& mapi_message::add_recipient(std::string email_address, std::string display_name, const int recipient_type)
{
    mapi_recipient recipient;
    recipient.email_address = std::move(email_address);
    recipient.display_name = display_name.empty() ? recipient.email_address : std::move(display_name);
    recipient.recipient_type = recipient_type;
    recipients_.push_back(std::move(recipient));
    return recipients_.back();
}

mapi_attachment& mapi_message::add_attachment(std::string filename, std::vector<std::uint8_t> data, std::string mime_type, std::string content_id)
{
    attachments_.push_back(mapi_attachment::from_bytes(std::move(filename), std::move(data), std::move(mime_type), std::move(content_id)));
    return attachments_.back();
}

mapi_attachment& mapi_message::add_attachment(std::string filename, std::istream& stream, std::string mime_type, std::string content_id)
{
    attachments_.push_back(mapi_attachment::from_stream(std::move(filename), stream, std::move(mime_type), std::move(content_id)));
    return attachments_.back();
}

mapi_attachment& mapi_message::add_embedded_message_attachment(mapi_message message, std::string filename, std::string mime_type)
{
    mapi_attachment attachment;
    attachment.filename = filename.empty() ? std::string("message.msg") : std::move(filename);
    attachment.mime_type = mime_type.empty() ? std::string("message/rfc822") : std::move(mime_type);
    attachment.embedded_message = std::make_shared<mapi_message>(std::move(message));
    attachments_.push_back(std::move(attachment));
    return attachments_.back();
}

mapi_property& mapi_message::set_property(const std::uint16_t property_id, const std::uint16_t property_type, std::any value, const std::uint32_t flags)
{
    return properties_.add(property_id, property_type, std::move(value), flags);
}

const std::any* mapi_message::get_property_value(const std::uint16_t property_id, const std::optional<std::uint16_t> property_type) const noexcept
{
    const auto* property = properties_.get(property_id, property_type);
    return property == nullptr ? nullptr : &property->value();
}

msg_document mapi_message::to_msg_document() const
{
    msg_storage root(std::string(cfb::cfb_constants::root_entry_name), msg_storage_role::message);
    auto properties = properties_;
    if (!message_class_.empty())
    {
        properties.add(to_underlying(common_message_property_id::message_class), to_underlying(property_type_code::ptyp_string), message_class_, default_property_flags);
    }

    if (!subject_.empty())
    {
        properties.add(to_underlying(common_message_property_id::subject), to_underlying(property_type_code::ptyp_string), subject_, default_property_flags);
    }

    if (!body_.empty())
    {
        properties.add(to_underlying(common_message_property_id::body), to_underlying(property_type_code::ptyp_string), body_, default_property_flags);
    }

    if (!html_body_.empty())
    {
        properties.add(to_underlying(common_message_property_id::body_html), to_underlying(property_type_code::ptyp_string), html_body_, default_property_flags);
    }

    if (!sender_name_.empty())
    {
        properties.add(to_underlying(common_message_property_id::sender_name), to_underlying(property_type_code::ptyp_string), sender_name_, default_property_flags);
    }

    if (!sender_email_address_.empty())
    {
        properties.add(to_underlying(common_message_property_id::sender_email_address), to_underlying(property_type_code::ptyp_string), sender_email_address_, default_property_flags);
    }

    if (!sender_address_type_.empty())
    {
        properties.add(to_underlying(common_message_property_id::sender_address_type), to_underlying(property_type_code::ptyp_string), sender_address_type_, default_property_flags);
    }

    if (!internet_message_id_.empty())
    {
        properties.add(to_underlying(common_message_property_id::internet_message_id), to_underlying(property_type_code::ptyp_string), internet_message_id_, default_property_flags);
    }

    properties.add(to_underlying(common_message_property_id::store_support_mask), to_underlying(property_type_code::ptyp_integer32), std::int32_t(0x00040000), default_property_flags);

    const auto serialized = serialize_property_bag(properties, msg_storage_role::message, recipients_.size(), attachments_.size());
    root.add_stream(serialized.property_stream);
    for (const auto& stream : serialized.value_streams)
    {
        root.add_stream(stream);
    }

    root.add_storage(msg_storage(std::string(msg_constants::named_property_mapping_storage_name), msg_storage_role::named_property_mapping));

    for (std::size_t index = 0; index < recipients_.size(); ++index)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "__recip_version1.0_#%08X", static_cast<unsigned>(index));
        msg_storage recipient_storage(buffer, msg_storage_role::recipient);
        auto recipient_props = recipients_[index].properties;
        recipient_props.add(display_name_property_id, to_underlying(property_type_code::ptyp_string), recipients_[index].display_name, default_property_flags);
        recipient_props.add(address_type_property_id, to_underlying(property_type_code::ptyp_string), recipients_[index].address_type, default_property_flags);
        recipient_props.add(email_address_property_id, to_underlying(property_type_code::ptyp_string), recipients_[index].email_address, default_property_flags);
        recipient_props.add(recipient_type_property_id, to_underlying(property_type_code::ptyp_integer32), recipients_[index].recipient_type, default_property_flags);
        const auto recipient_serialized = serialize_property_bag(recipient_props, msg_storage_role::recipient, 0U, 0U);
        recipient_storage.add_stream(recipient_serialized.property_stream);
        for (const auto& stream : recipient_serialized.value_streams)
        {
            recipient_storage.add_stream(stream);
        }

        root.add_storage(std::move(recipient_storage));
    }

    for (std::size_t index = 0; index < attachments_.size(); ++index)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "__attach_version1.0_#%08X", static_cast<unsigned>(index));
        msg_storage attachment_storage(buffer, msg_storage_role::attachment);
        auto attachment_props = attachments_[index].properties;
        if (!attachments_[index].filename.empty())
        {
            attachment_props.add(to_underlying(common_message_property_id::attach_long_filename), to_underlying(property_type_code::ptyp_string), attachments_[index].filename, default_property_flags);
            attachment_props.add(to_underlying(common_message_property_id::attach_filename), to_underlying(property_type_code::ptyp_string), short_filename(attachments_[index].filename), default_property_flags);
        }

        if (!attachments_[index].mime_type.empty())
        {
            attachment_props.add(to_underlying(common_message_property_id::attach_mime_tag), to_underlying(property_type_code::ptyp_string), attachments_[index].mime_type, default_property_flags);
        }

        if (!attachments_[index].content_id.empty())
        {
            attachment_props.add(to_underlying(common_message_property_id::attach_content_id), to_underlying(property_type_code::ptyp_string), attachments_[index].content_id, default_property_flags);
        }

        if (attachments_[index].is_embedded_message())
        {
            attachment_props.add(to_underlying(common_message_property_id::attach_method), to_underlying(property_type_code::ptyp_integer32), attach_method_embedded, default_property_flags);
            attachment_props.add(to_underlying(common_message_property_id::attach_data_binary), to_underlying(property_type_code::ptyp_object), std::any {}, default_property_flags);
        }
        else
        {
            attachment_props.add(to_underlying(common_message_property_id::attach_method), to_underlying(property_type_code::ptyp_integer32), attach_method_by_value, default_property_flags);
            attachment_props.add(to_underlying(common_message_property_id::attach_data_binary), to_underlying(property_type_code::ptyp_binary), attachments_[index].data, default_property_flags);
        }

        const auto attachment_serialized = serialize_property_bag(attachment_props, msg_storage_role::attachment, 0U, 0U);
        attachment_storage.add_stream(attachment_serialized.property_stream);
        for (const auto& stream : attachment_serialized.value_streams)
        {
            attachment_storage.add_stream(stream);
        }

        if (attachments_[index].is_embedded_message())
        {
            auto embedded = attachments_[index].embedded_message->to_msg_document().root();
            embedded.name = std::string(msg_constants::embedded_message_storage_name);
            embedded.role = msg_storage_role::embedded_message;
            attachment_storage.add_storage(std::move(embedded));
        }

        root.add_storage(std::move(attachment_storage));
    }

    return msg_document(std::move(root));
}

std::vector<std::uint8_t> mapi_message::save() const
{
    return msg_writer::to_bytes(to_msg_document());
}

void mapi_message::save(const std::filesystem::path& path) const
{
    std::ofstream stream(path, std::ios::binary);
    if (!stream)
    {
        throw msg_exception("Unable to open output path for writing: " + path.string());
    }

    save(stream);
}

void mapi_message::save(std::ostream& stream) const
{
    const auto bytes = save();
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!stream)
    {
        throw msg_exception("Failed to write MSG payload.");
    }
}

std::vector<std::uint8_t> mapi_message::save_to_eml() const
{
    mime::mime_writer writer;
    return writer.write(mime::eml_message_mapper::to_mime_message(*this));
}

void mapi_message::save_to_eml(const std::filesystem::path& path) const
{
    std::ofstream stream(path, std::ios::binary);
    if (!stream)
    {
        throw msg_exception("Unable to open EML output path for writing: " + path.string());
    }

    save_to_eml(stream);
}

void mapi_message::save_to_eml(std::ostream& stream) const
{
    const auto bytes = save_to_eml();
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!stream)
    {
        throw msg_exception("Failed to write EML payload.");
    }
}

} // namespace aspose::email::foss::msg
