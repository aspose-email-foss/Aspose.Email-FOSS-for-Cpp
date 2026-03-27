#pragma once

#include <any>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "aspose/email/foss/msg/mapi_attachment.hpp"
#include "aspose/email/foss/msg/msg_constants.hpp"
#include "aspose/email/foss/msg/mapi_property_collection.hpp"
#include "aspose/email/foss/msg/mapi_recipient.hpp"
#include "aspose/email/foss/msg/msg_document.hpp"

namespace aspose::email::foss::msg
{

class mapi_message
{
public:
    static constexpr std::uint32_t default_property_flags = msg_constants::propattr_readable | msg_constants::propattr_writable;
    static constexpr int recipient_type_to = 1;
    static constexpr int recipient_type_cc = 2;
    static constexpr int recipient_type_bcc = 3;
    static constexpr int attach_method_by_value = 1;
    static constexpr int attach_method_embedded = 5;

    [[nodiscard]] static mapi_message create(std::string subject = {}, std::string body = {}, bool unicode_strings = true);
    [[nodiscard]] static mapi_message from_file(const std::filesystem::path& path, bool strict = false);
    [[nodiscard]] static mapi_message from_stream(std::istream& stream, bool strict = false);
    [[nodiscard]] static mapi_message from_msg_document(const msg_document& document);
    [[nodiscard]] static mapi_message load_from_eml(const std::filesystem::path& path);
    [[nodiscard]] static mapi_message load_from_eml(std::istream& stream);

    [[nodiscard]] bool unicode_strings() const noexcept;
    void set_unicode_strings(bool value) noexcept;

    [[nodiscard]] const std::vector<std::string>& validation_issues() const noexcept;

    [[nodiscard]] const std::string& subject() const noexcept;
    void set_subject(std::string value);

    [[nodiscard]] const std::string& body() const noexcept;
    void set_body(std::string value);

    [[nodiscard]] const std::string& html_body() const noexcept;
    void set_html_body(std::string value);

    [[nodiscard]] const std::string& message_class() const noexcept;
    void set_message_class(std::string value);

    [[nodiscard]] const std::string& sender_name() const noexcept;
    void set_sender_name(std::string value);

    [[nodiscard]] const std::string& sender_email_address() const noexcept;
    void set_sender_email_address(std::string value);

    [[nodiscard]] const std::string& sender_address_type() const noexcept;
    void set_sender_address_type(std::string value);

    [[nodiscard]] const std::string& internet_message_id() const noexcept;
    void set_internet_message_id(std::string value);

    [[nodiscard]] mapi_property_collection& properties() noexcept;
    [[nodiscard]] const mapi_property_collection& properties() const noexcept;
    [[nodiscard]] std::vector<mapi_recipient>& recipients() noexcept;
    [[nodiscard]] const std::vector<mapi_recipient>& recipients() const noexcept;
    [[nodiscard]] std::vector<mapi_attachment>& attachments() noexcept;
    [[nodiscard]] const std::vector<mapi_attachment>& attachments() const noexcept;

    mapi_recipient& add_recipient(std::string email_address, std::string display_name = {}, int recipient_type = recipient_type_to);
    mapi_attachment& add_attachment(std::string filename, std::vector<std::uint8_t> data, std::string mime_type = {}, std::string content_id = {});
    mapi_attachment& add_attachment(std::string filename, std::istream& stream, std::string mime_type = {}, std::string content_id = {});
    mapi_attachment& add_embedded_message_attachment(mapi_message message, std::string filename = {}, std::string mime_type = {});

    mapi_property& set_property(std::uint16_t property_id, std::uint16_t property_type, std::any value, std::uint32_t flags = default_property_flags);
    [[nodiscard]] const std::any* get_property_value(std::uint16_t property_id, std::optional<std::uint16_t> property_type = std::nullopt) const noexcept;

    [[nodiscard]] msg_document to_msg_document() const;
    [[nodiscard]] std::vector<std::uint8_t> save() const;
    void save(const std::filesystem::path& path) const;
    void save(std::ostream& stream) const;
    [[nodiscard]] std::vector<std::uint8_t> save_to_eml() const;
    void save_to_eml(const std::filesystem::path& path) const;
    void save_to_eml(std::ostream& stream) const;

private:
    bool unicode_strings_ {true};
    mapi_property_collection properties_;
    std::vector<mapi_recipient> recipients_;
    std::vector<mapi_attachment> attachments_;
    std::vector<std::string> validation_issues_;
    std::string subject_;
    std::string body_;
    std::string html_body_;
    std::string message_class_ {"IPM.Note"};
    std::string sender_name_;
    std::string sender_email_address_;
    std::string sender_address_type_ {"SMTP"};
    std::string internet_message_id_;
};

} // namespace aspose::email::foss::msg
