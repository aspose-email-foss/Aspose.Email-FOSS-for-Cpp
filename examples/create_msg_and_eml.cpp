#include <cstdint>
#include <filesystem>
#include <iostream>
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

std::string read_option(const int argc, char* argv[], const std::string& name, const std::string& fallback)
{
    for (int index = 1; index + 1 < argc; ++index)
    {
        if (name == argv[index])
        {
            return argv[index + 1];
        }
    }

    return fallback;
}

} // namespace

int main(int argc, char* argv[])
{
    const auto msg_path = read_option(argc, argv, "--msg-path", "example-message.msg");
    const auto eml_path = read_option(argc, argv, "--eml-path", "example-message.eml");

    auto message = aspose::email::foss::msg::mapi_message::create(
        "Quarterly status update and rollout plan",
        "Hello team,\n\nPlease find the latest rollout summary attached.\n\nRegards,\nEngineering");

    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::sender_name),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("Build Agent"));
    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::sender_email_address),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("build.agent@example.com"));
    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::internet_message_id),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("<example-message-001@example.com>"));
    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::display_to),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("Alice Example; Bob Example"));
    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::display_cc),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("Carol Example"));
    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::display_bcc),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("Ops Archive"));
    message.set_property(
        to_underlying(aspose::email::foss::msg::common_message_property_id::transport_message_headers),
        to_underlying(aspose::email::foss::msg::property_type_code::ptyp_string),
        std::string("X-Environment: example\r\nX-Workflow: create-msg-and-eml\r\n"));

    message.add_recipient("alice@example.com", "Alice Example");
    message.add_recipient("bob@example.com", "Bob Example");
    message.add_recipient("carol@example.com", "Carol Example", aspose::email::foss::msg::mapi_message::recipient_type_cc);
    message.add_recipient("archive@example.com", "Ops Archive", aspose::email::foss::msg::mapi_message::recipient_type_bcc);

    message.add_attachment("hello.txt", std::vector<std::uint8_t> {'s', 'a', 'm', 'p', 'l', 'e', ' ', 'a', 't', 't', 'a', 'c', 'h', 'm', 'e', 'n', 't', '\n'}, "text/plain");
    message.add_attachment("report.bin", std::vector<std::uint8_t> {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}, "application/octet-stream");
    message.save(std::filesystem::path(msg_path));

    auto loaded_message = aspose::email::foss::msg::mapi_message::from_file(std::filesystem::path(msg_path));
    loaded_message.save_to_eml(std::filesystem::path(eml_path));

    std::cout << "MSG saved to: " << msg_path << '\n';
    std::cout << "EML saved to: " << eml_path << '\n';
    return 0;
}
