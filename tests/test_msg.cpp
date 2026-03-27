#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "aspose/email/foss/msg/mapi_message.hpp"
#include "aspose/email/foss/msg/msg_document.hpp"

namespace
{

int require(const bool condition, const char* message)
{
    if (condition)
    {
        return 0;
    }

    std::cerr << message << '\n';
    return 1;
}

} // namespace

int main()
{
    using namespace aspose::email::foss::msg;

    auto embedded = mapi_message::create("Embedded", "Nested body");
    auto message = mapi_message::create("Hello", "Body text");
    message.set_sender_name("Alice");
    message.set_sender_email_address("alice@example.com");
    message.add_recipient("bob@example.com", "Bob");
    message.add_attachment("note.txt", std::vector<std::uint8_t> {'a', 'b', 'c'}, "text/plain", "cid-note");
    message.add_embedded_message_attachment(embedded, "nested.msg");

    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    message.save(stream);
    stream.seekg(0, std::ios::beg);

    auto loaded = mapi_message::from_stream(stream);
    if (auto rc = require(loaded.subject() == "Hello", "Subject round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(loaded.body() == "Body text", "Body round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(loaded.recipients().size() == 1U, "Recipient round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(loaded.attachments().size() == 2U, "Attachment round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(loaded.attachments()[0].data == std::vector<std::uint8_t>({'a', 'b', 'c'}), "Attachment payload round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(loaded.attachments()[1].is_embedded_message(), "Embedded attachment flag round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(loaded.attachments()[1].embedded_message->subject() == "Embedded", "Embedded message subject round-trip failed"); rc != 0)
    {
        return rc;
    }

    auto document = loaded.to_msg_document();
    if (auto rc = require(document.root().find_stream("__properties_version1.0") != nullptr, "MSG document root property stream missing"); rc != 0)
    {
        return rc;
    }

    std::stringstream eml_stream(std::ios::in | std::ios::out | std::ios::binary);
    loaded.save_to_eml(eml_stream);
    eml_stream.seekg(0, std::ios::beg);
    auto from_eml = mapi_message::load_from_eml(eml_stream);
    if (auto rc = require(from_eml.subject() == "Hello", "EML subject round-trip failed"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(from_eml.attachments().size() == 2U, "EML attachment round-trip failed"); rc != 0)
    {
        return rc;
    }

    return EXIT_SUCCESS;
}
