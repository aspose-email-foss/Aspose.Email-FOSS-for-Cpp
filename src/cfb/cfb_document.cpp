#include "aspose/email/foss/cfb/cfb_document.hpp"

#include <utility>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/cfb/cfb_reader.hpp"
#include "aspose/email/foss/cfb/cfb_stream.hpp"

namespace aspose::email::foss::cfb
{

cfb_document::cfb_document()
    : cfb_document(cfb_storage(std::string(cfb_constants::root_entry_name)))
{
}

cfb_document::cfb_document(
    cfb_storage root,
    const std::uint16_t major_version,
    const std::uint16_t minor_version,
    const std::uint32_t transaction_signature_number)
    : root_(std::move(root)),
      major_version_(major_version),
      minor_version_(minor_version),
      transaction_signature_number_(transaction_signature_number)
{
}

cfb_storage& cfb_document::root() noexcept
{
    return root_;
}

const cfb_storage& cfb_document::root() const noexcept
{
    return root_;
}

std::uint16_t cfb_document::major_version() const noexcept
{
    return major_version_;
}

void cfb_document::set_major_version(const std::uint16_t value) noexcept
{
    major_version_ = value;
}

std::uint16_t cfb_document::minor_version() const noexcept
{
    return minor_version_;
}

void cfb_document::set_minor_version(const std::uint16_t value) noexcept
{
    minor_version_ = value;
}

std::uint32_t cfb_document::transaction_signature_number() const noexcept
{
    return transaction_signature_number_;
}

void cfb_document::set_transaction_signature_number(const std::uint32_t value) noexcept
{
    transaction_signature_number_ = value;
}

cfb_document cfb_document::from_reader(const cfb_reader& reader)
{
    const auto build_storage = [&reader](const auto& self, const std::uint32_t stream_id) -> cfb_storage
    {
        const auto& entry = reader.get_entry(stream_id);
        cfb_storage storage(entry.name);
        storage.clsid() = entry.clsid;
        storage.set_state_bits(entry.state_bits);
        storage.set_creation_time(entry.creation_time);
        storage.set_modified_time(entry.modified_time);

        for (const auto child_id : reader.child_ids(stream_id))
        {
            const auto& child = reader.get_entry(child_id);
            if (child.is_storage())
            {
                storage.add_storage(self(self, child.stream_id));
                continue;
            }

            if (child.is_stream())
            {
                cfb_stream stream(child.name, reader.get_stream_data(child.stream_id));
                stream.clsid() = child.clsid;
                stream.set_state_bits(child.state_bits);
                stream.set_creation_time(child.creation_time);
                stream.set_modified_time(child.modified_time);
                storage.add_stream(std::move(stream));
            }
        }

        return storage;
    };

    return cfb_document(
        build_storage(build_storage, cfb_constants::root_stream_id),
        reader.header().major_version,
        reader.header().minor_version,
        reader.header().transaction_signature_number);
}

cfb_document cfb_document::from_file(const std::filesystem::path& path)
{
    return from_reader(cfb_reader::from_file(path));
}

cfb_document cfb_document::from_stream(std::istream& stream)
{
    return from_reader(cfb_reader::from_stream(stream));
}

cfb_document cfb_document::from_bytes(std::vector<std::uint8_t> data)
{
    return from_reader(cfb_reader::from_bytes(std::move(data)));
}

cfb_document cfb_document::from_buffer(const std::uint8_t* data, const std::size_t size)
{
    return from_reader(cfb_reader::from_buffer(data, size));
}

} // namespace aspose::email::foss::cfb
