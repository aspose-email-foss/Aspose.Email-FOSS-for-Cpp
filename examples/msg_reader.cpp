#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "aspose/email/foss/cfb/cfb_reader.hpp"
#include "aspose/email/foss/cfb/directory_entry.hpp"
#include "aspose/email/foss/msg/msg_constants.hpp"
#include "aspose/email/foss/msg/msg_document.hpp"
#include "aspose/email/foss/msg/msg_reader.hpp"
#include "aspose/email/foss/msg/msg_storage_role.hpp"

namespace
{

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

std::string entry_type_name(const aspose::email::foss::cfb::directory_entry& entry)
{
    if (entry.is_root())
    {
        return "root";
    }

    if (entry.is_storage())
    {
        return "storage";
    }

    if (entry.is_stream())
    {
        return "stream";
    }

    return "unknown";
}

void dump_cfb_tree(
    const aspose::email::foss::cfb::cfb_reader& cfb,
    const std::uint32_t stream_id,
    const int depth,
    std::vector<std::string>& lines)
{
    const auto& entry = cfb.get_entry(stream_id);
    const std::string indent(static_cast<std::size_t>(depth * 2), ' ');
    std::ostringstream line;
    line << indent << "- " << entry.name << " [sid=" << entry.stream_id << ", type=" << entry_type_name(entry);
    if (entry.is_root() || entry.is_stream())
    {
        line << ", size=" << entry.stream_size;
    }

    line << "]";
    lines.push_back(line.str());

    for (const auto child_id : cfb.child_ids(stream_id))
    {
        dump_cfb_tree(cfb, child_id, depth + 1, lines);
    }
}

std::string build_dump(const aspose::email::foss::msg::msg_reader& reader, const aspose::email::foss::msg::msg_document& document, const std::filesystem::path& msg_path)
{
    const auto& cfb = reader.cfb();
    const auto& root = document.root();
    std::vector<const aspose::email::foss::msg::msg_storage*> recipients;
    std::vector<const aspose::email::foss::msg::msg_storage*> attachments;
    const aspose::email::foss::msg::msg_storage* named_property_mapping = nullptr;

    for (const auto& storage : root.storages)
    {
        if (storage.role == aspose::email::foss::msg::msg_storage_role::recipient)
        {
            recipients.push_back(&storage);
        }
        else if (storage.role == aspose::email::foss::msg::msg_storage_role::attachment)
        {
            attachments.push_back(&storage);
        }
        else if (storage.role == aspose::email::foss::msg::msg_storage_role::named_property_mapping)
        {
            named_property_mapping = &storage;
        }
    }

    const auto* property_stream = root.find_stream(std::string(aspose::email::foss::msg::msg_constants::property_stream_name));

    std::vector<std::string> lines {
        "MSG file: " + msg_path.string(),
        "Compound File Binary (CFB) Summary:",
        "- major_version=" + std::to_string(cfb.header().major_version),
        "- sector_size=" + std::to_string(cfb.header().sector_size()),
        "- mini_sector_size=" + std::to_string(cfb.header().mini_sector_size()),
        "- fat_sectors=" + std::to_string(cfb.fat().size()),
        "- directory_entries=" + std::to_string(cfb.directory_entry_count()),
        "- streams_materialized=" + std::to_string(cfb.materialized_stream_count()),
        "- file_size=" + std::to_string(cfb.data_size()),
        "MSG Summary:",
        "- validation_issues=" + std::to_string(reader.validation_issues().size()),
        "- named_property_mapping=" + std::string(named_property_mapping == nullptr ? "<missing>" : named_property_mapping->name),
        "- top_level_property_stream=" + std::string(property_stream == nullptr ? "<missing>" : property_stream->name),
        "- top_level_property_stream_size=" + std::to_string(property_stream == nullptr ? 0U : property_stream->data.size()),
        "- recipients=" + std::to_string(recipients.size()),
        "- attachments=" + std::to_string(attachments.size()),
    };

    if (!reader.validation_issues().empty())
    {
        lines.push_back("Validation Issues:");
        for (const auto& issue : reader.validation_issues())
        {
            lines.push_back("- " + issue);
        }
    }

    lines.push_back("Recipient Storages:");
    if (recipients.empty())
    {
        lines.push_back("- <none>");
    }
    else
    {
        for (const auto* recipient : recipients)
        {
            const auto* recipient_property_stream = recipient->find_stream(std::string(aspose::email::foss::msg::msg_constants::property_stream_name));
            lines.push_back(
                "- " + recipient->name +
                " role=" + std::to_string(static_cast<int>(recipient->role)) +
                " property_stream_size=" + std::to_string(recipient_property_stream == nullptr ? 0U : recipient_property_stream->data.size()) +
                " nested_storages=" + std::to_string(recipient->storages.size()));
        }
    }

    lines.push_back("Attachment Storages:");
    if (attachments.empty())
    {
        lines.push_back("- <none>");
    }
    else
    {
        for (const auto* attachment : attachments)
        {
            const auto* attachment_property_stream = attachment->find_stream(std::string(aspose::email::foss::msg::msg_constants::property_stream_name));
            const auto* embedded = attachment->find_storage(std::string(aspose::email::foss::msg::msg_constants::embedded_message_storage_name));
            lines.push_back(
                "- " + attachment->name +
                " role=" + std::to_string(static_cast<int>(attachment->role)) +
                " property_stream_size=" + std::to_string(attachment_property_stream == nullptr ? 0U : attachment_property_stream->data.size()) +
                " embedded_message=" + std::string(embedded == nullptr ? "false" : "true"));
        }
    }

    lines.push_back("Compound File Binary (CFB) Tree:");
    dump_cfb_tree(cfb, 0U, 0, lines);

    std::ostringstream result;
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        if (index != 0U)
        {
            result << '\n';
        }

        result << lines[index];
    }

    return result.str();
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: msg_reader.cpp <path-to-msg> [--out <path>]\n";
        return 1;
    }

    const std::filesystem::path msg_path(argv[1]);
    const auto out_path = read_option(argc, argv, "--out");

    const auto reader = aspose::email::foss::msg::msg_reader::from_file(msg_path);
    const auto document = aspose::email::foss::msg::msg_document::from_reader(reader);
    const auto dump = build_dump(reader, document, msg_path);

    if (!out_path.empty())
    {
        std::ofstream output(out_path, std::ios::binary);
        output << dump;
    }
    else
    {
        std::cout << dump << '\n';
    }

    return 0;
}
