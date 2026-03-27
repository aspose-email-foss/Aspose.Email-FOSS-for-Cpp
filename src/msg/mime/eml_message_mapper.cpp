#include "eml_message_mapper.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

#include "../../cfb/detail.hpp"
#include "aspose/email/foss/msg/mapi_message.hpp"
#include "mime_writer.hpp"

namespace aspose::email::foss::msg::mime
{

namespace
{

[[nodiscard]] std::string lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character)
    {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

[[nodiscard]] std::string header_value(const std::vector<mime_header>& headers, const std::string& name)
{
    const auto target = lower(name);
    for (const auto& header : headers)
    {
        if (lower(header.name) == target)
        {
            return header.value;
        }
    }

    return {};
}

[[nodiscard]] std::pair<std::string, std::string> parse_mailbox(const std::string& value)
{
    const auto lt = value.find('<');
    const auto gt = value.find('>');
    if (lt != std::string::npos && gt != std::string::npos && gt > lt)
    {
        return {value.substr(0U, lt), value.substr(lt + 1U, gt - lt - 1U)};
    }

    return {value, value};
}

void collect_body_and_attachments(
    const mime_entity& entity,
    std::string& body,
    std::string& html_body,
    std::vector<mapi_attachment>& attachments)
{
    if (entity.is_multipart())
    {
        for (const auto& child : entity.children)
        {
            collect_body_and_attachments(child, body, html_body, attachments);
        }

        return;
    }

    const auto is_attachment = entity.content_disposition.find("attachment") != std::string::npos || !entity.filename.empty();
    if (is_attachment)
    {
        auto attachment = mapi_attachment::from_bytes(entity.filename.empty() ? std::string("attachment.bin") : entity.filename, entity.body);
        attachment.content_id = entity.content_id;
        attachment.mime_type = entity.content_type;
        attachments.push_back(std::move(attachment));
        return;
    }

    if (entity.content_type == "text/html")
    {
        html_body.assign(entity.body.begin(), entity.body.end());
        return;
    }

    body.assign(entity.body.begin(), entity.body.end());
}

} // namespace

mapi_message eml_message_mapper::to_mapi_message(const mime_message_model& model)
{
    mapi_message message = mapi_message::create();
    message.set_subject(header_value(model.headers, "Subject"));
    message.set_internet_message_id(header_value(model.headers, "Message-ID"));

    const auto [sender_name, sender_email] = parse_mailbox(header_value(model.headers, "From"));
    message.set_sender_name(sender_name);
    message.set_sender_email_address(sender_email);

    const auto to_header = header_value(model.headers, "To");
    if (!to_header.empty())
    {
        const auto [display_name, email] = parse_mailbox(to_header);
        message.add_recipient(email, display_name, mapi_message::recipient_type_to);
    }

    const auto cc_header = header_value(model.headers, "Cc");
    if (!cc_header.empty())
    {
        const auto [display_name, email] = parse_mailbox(cc_header);
        message.add_recipient(email, display_name, mapi_message::recipient_type_cc);
    }

    std::string body;
    std::string html_body;
    std::vector<mapi_attachment> attachments;
    collect_body_and_attachments(model.root, body, html_body, attachments);
    message.set_body(body);
    message.set_html_body(html_body);
    for (auto& attachment : attachments)
    {
        message.attachments().push_back(std::move(attachment));
    }

    return message;
}

mime_message_model eml_message_mapper::to_mime_message(const mapi_message& message)
{
    mime_message_model model;
    model.headers.push_back({"MIME-Version", "1.0"});
    if (!message.sender_email_address().empty())
    {
        model.headers.push_back({"From", message.sender_name().empty() ? message.sender_email_address() : message.sender_name() + " <" + message.sender_email_address() + ">"});
    }

    for (const auto& recipient : message.recipients())
    {
        if (recipient.recipient_type == mapi_message::recipient_type_to)
        {
            model.headers.push_back({"To", recipient.display_name.empty() ? recipient.email_address : recipient.display_name + " <" + recipient.email_address + ">"});
            break;
        }
    }

    model.headers.push_back({"Subject", message.subject()});
    if (!message.internet_message_id().empty())
    {
        model.headers.push_back({"Message-ID", message.internet_message_id()});
    }

    if (message.attachments().empty())
    {
        model.headers.push_back({"Content-Type", message.html_body().empty() ? "text/plain; charset=utf-8" : "text/html; charset=utf-8"});
        model.headers.push_back({"Content-Transfer-Encoding", "quoted-printable"});
        model.root.content_type = std::string(message.html_body().empty() ? "text/plain" : "text/html");
        model.root.content_transfer_encoding = "quoted-printable";
        const auto& body = message.html_body().empty() ? message.body() : message.html_body();
        model.root.body.assign(body.begin(), body.end());
        return model;
    }

    const auto boundary = mime_boundary_factory::create();
    model.headers.push_back({"Content-Type", "multipart/mixed; boundary=\"" + boundary + "\""});
    model.root.content_type = "multipart/mixed";
    model.root.boundary = boundary;

    mime_entity body_part;
    body_part.content_type = std::string(message.html_body().empty() ? "text/plain" : "text/html");
    body_part.content_transfer_encoding = "quoted-printable";
    body_part.headers.push_back({"Content-Type", body_part.content_type + "; charset=utf-8"});
    body_part.headers.push_back({"Content-Transfer-Encoding", "quoted-printable"});
    const auto& body = message.html_body().empty() ? message.body() : message.html_body();
    body_part.body.assign(body.begin(), body.end());
    model.root.children.push_back(std::move(body_part));

    for (const auto& attachment : message.attachments())
    {
        mime_entity part;
        part.content_type = attachment.mime_type.empty() ? (attachment.is_embedded_message() ? "application/vnd.ms-outlook" : "application/octet-stream") : attachment.mime_type;
        part.content_transfer_encoding = "base64";
        part.content_disposition = "attachment";
        part.filename = attachment.filename;
        part.content_id = attachment.content_id;
        part.headers.push_back({"Content-Type", part.content_type + "; name=\"" + part.filename + "\""});
        part.headers.push_back({"Content-Disposition", "attachment; filename=\"" + part.filename + "\""});
        if (!part.content_id.empty())
        {
            part.headers.push_back({"Content-ID", part.content_id});
        }

        part.headers.push_back({"Content-Transfer-Encoding", "base64"});
        if (attachment.is_embedded_message())
        {
            part.body = attachment.embedded_message->save();
        }
        else
        {
            part.body = attachment.data;
        }

        model.root.children.push_back(std::move(part));
    }

    return model;
}

} // namespace aspose::email::foss::msg::mime
