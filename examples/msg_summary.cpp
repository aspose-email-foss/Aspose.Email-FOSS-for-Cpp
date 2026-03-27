#include <algorithm>
#include <any>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "aspose/email/foss/msg/common_message_property_id.hpp"
#include "aspose/email/foss/msg/mapi_message.hpp"
#include "aspose/email/foss/msg/property_type_code.hpp"

namespace
{

template <typename Enum>
constexpr auto to_underlying(const Enum value) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(value);
}

std::string read_option(const int argc, char* argv[], const std::string& name)
{
    for (int index = 1; index + 1 < argc; ++index)
    {
        if (name == argv[index])
        {
            return argv[index + 1];
        }
    }

    return {};
}

std::uint16_t parse_u16(const std::string& text)
{
    const int base = text.rfind("0x", 0) == 0 || text.rfind("0X", 0) == 0 ? 16 : 10;
    return static_cast<std::uint16_t>(std::stoul(text, nullptr, base));
}

std::string body_preview(const std::string& body, const std::size_t limit)
{
    return body.size() <= limit ? body : body.substr(0U, limit);
}

std::string any_string(const std::any* value)
{
    const auto* text = value == nullptr ? nullptr : std::any_cast<std::string>(value);
    return text == nullptr ? std::string {} : *text;
}

void print_transport_headers(const aspose::email::foss::msg::mapi_message& message)
{
    std::cout << "\nTransport Headers:\n";
    const auto* value = message.get_property_value(
        to_underlying(aspose::email::foss::msg::common_message_property_id::transport_message_headers),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string));
    const auto text = any_string(value);
    if (text.empty())
    {
        std::cout << "- <none>\n";
        return;
    }

    std::size_t start = 0U;
    while (start < text.size())
    {
        const auto end = text.find_first_of("\r\n", start);
        const auto line = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (!line.empty())
        {
            std::cout << "- " << line << '\n';
        }

        if (end == std::string::npos)
        {
            break;
        }

        const auto next = text.find_first_not_of("\r\n", end);
        if (next == std::string::npos)
        {
            break;
        }

        start = next;
    }
}

void print_property_lookup(
    const aspose::email::foss::msg::mapi_message& message,
    const std::uint16_t property_id,
    const std::optional<std::uint16_t> property_type)
{
    const auto* value = message.get_property_value(property_id, property_type);
    std::cout << "\nArbitrary Property Lookup:\n";
    std::cout << "- property_id=0x" << std::hex << std::uppercase << property_id << std::dec << '\n';
    if (property_type.has_value())
    {
        std::cout << "- property_type=0x" << std::hex << std::uppercase << *property_type << std::dec << '\n';
    }
    else
    {
        std::cout << "- property_type=<auto>\n";
    }

    if (value == nullptr)
    {
        std::cout << "- value=<not found>\n";
        return;
    }

    if (const auto* text = std::any_cast<std::string>(value))
    {
        std::cout << "- value_type=std::string\n";
        std::cout << "- value=" << *text << '\n';
        return;
    }

    if (const auto* bytes = std::any_cast<std::vector<std::uint8_t>>(value))
    {
        std::cout << "- value_type=std::vector<std::uint8_t>\n";
        std::cout << "- value_len=" << bytes->size() << '\n';
        std::cout << "- value_hex=";
        const std::size_t limit = std::min<std::size_t>(bytes->size(), 32U);
        static const char hex[] = "0123456789ABCDEF";
        for (std::size_t index = 0; index < limit; ++index)
        {
            const auto byte = (*bytes)[index];
            std::cout << hex[(byte >> 4) & 0x0F] << hex[byte & 0x0F];
        }

        if (bytes->size() > limit)
        {
            std::cout << "...";
        }

        std::cout << '\n';
        return;
    }

    if (const auto* value32 = std::any_cast<std::int32_t>(value))
    {
        std::cout << "- value_type=int32_t\n";
        std::cout << "- value=" << *value32 << '\n';
        return;
    }

    if (const auto* value64 = std::any_cast<std::uint64_t>(value))
    {
        std::cout << "- value_type=uint64_t\n";
        std::cout << "- value=" << *value64 << '\n';
        return;
    }

    if (const auto* boolean = std::any_cast<bool>(value))
    {
        std::cout << "- value_type=bool\n";
        std::cout << "- value=" << (*boolean ? "true" : "false") << '\n';
        return;
    }

    std::cout << "- value_type=<unsupported>\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: msg_summary.cpp <path-to-msg> [--body-preview-chars <count>] [--property-id <id>] [--property-type <type>]\n";
        return 1;
    }

    const std::filesystem::path msg_path(argv[1]);
    const auto body_limit_text = read_option(argc, argv, "--body-preview-chars");
    const auto property_id_text = read_option(argc, argv, "--property-id");
    const auto property_type_text = read_option(argc, argv, "--property-type");
    const auto body_limit = body_limit_text.empty() ? 400U : static_cast<std::size_t>(std::stoul(body_limit_text));

    const auto message = aspose::email::foss::msg::mapi_message::from_file(msg_path);

    std::cout << "Message Summary:\n";
    std::cout << "file: " << msg_path.string() << '\n';
    std::cout << "subject: " << message.subject() << '\n';
    std::cout << "message_class: " << message.message_class() << '\n';
    std::cout << "sender_name: " << message.sender_name() << '\n';
    std::cout << "sender_email: " << message.sender_email_address() << '\n';
    std::cout << "internet_message_id: " << message.internet_message_id() << '\n';
    std::cout << "recipients_count: " << message.recipients().size() << '\n';
    std::cout << "attachments_count: " << message.attachments().size() << '\n';
    std::cout << "validation_issues: " << message.validation_issues().size() << '\n';

    std::cout << "\nDisplay Recipients:\n";
    std::cout << "- To: " << any_string(message.get_property_value(
        to_underlying(aspose::email::foss::msg::common_message_property_id::display_to),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string))) << '\n';
    std::cout << "- Cc: " << any_string(message.get_property_value(
        to_underlying(aspose::email::foss::msg::common_message_property_id::display_cc),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string))) << '\n';
    std::cout << "- Bcc: " << any_string(message.get_property_value(
        to_underlying(aspose::email::foss::msg::common_message_property_id::display_bcc),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string))) << '\n';

    print_transport_headers(message);

    std::cout << "\nRecipients:\n";
    if (message.recipients().empty())
    {
        std::cout << "- <none>\n";
    }
    else
    {
        for (std::size_t index = 0; index < message.recipients().size(); ++index)
        {
            const auto& recipient = message.recipients()[index];
            std::cout
                << "- [" << (index + 1U) << "] name=" << recipient.display_name
                << " email=" << recipient.email_address
                << " type=" << recipient.recipient_type
                << '\n';
        }
    }

    std::cout << "\nBody Preview:\n";
    const auto& body = message.html_body().empty() ? message.body() : message.html_body();
    const auto preview = body_preview(body, body_limit);
    std::cout << preview << '\n';
    if (body.size() > preview.size())
    {
        std::cout << "... (" << (body.size() - preview.size()) << " more chars omitted)\n";
    }

    std::cout << "\nAttachments:\n";
    if (message.attachments().empty())
    {
        std::cout << "- <none>\n";
    }
    else
    {
        for (std::size_t index = 0; index < message.attachments().size(); ++index)
        {
            const auto& attachment = message.attachments()[index];
            std::cout
                << "- [" << (index + 1U) << "] name=" << attachment.filename
                << " mime=" << attachment.mime_type
                << " size=" << attachment.data.size()
                << " content_id=" << attachment.content_id
                << " embedded=" << (attachment.is_embedded_message() ? "true" : "false")
                << '\n';
        }
    }

    std::cout << "\nAttachment Memory Read Check:\n";
    if (message.attachments().empty())
    {
        std::cout << "- <none>\n";
    }
    else
    {
        for (std::size_t index = 0; index < message.attachments().size(); ++index)
        {
            const auto& attachment = message.attachments()[index];
            const auto bytes_in_memory = attachment.is_embedded_message() && attachment.embedded_message != nullptr
                ? attachment.embedded_message->save().size()
                : attachment.data.size();
            std::cout
                << "- [" << (index + 1U) << "] name=" << attachment.filename
                << " read_ok=true bytes_in_memory=" << bytes_in_memory
                << " content_type=" << attachment.mime_type
                << '\n';
        }
    }

    if (!property_id_text.empty())
    {
        const auto property_id = parse_u16(property_id_text);
        const auto property_type = property_type_text.empty()
            ? std::optional<std::uint16_t> {}
            : std::optional<std::uint16_t> {parse_u16(property_type_text)};
        print_property_lookup(message, property_id, property_type);
    }

    return 0;
}
