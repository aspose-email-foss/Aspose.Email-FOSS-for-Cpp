#include "aspose/email/foss/msg/msg_reader.hpp"

#include <regex>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/msg/msg_constants.hpp"
#include "aspose/email/foss/msg/msg_exception.hpp"

namespace aspose::email::foss::msg
{

namespace
{

const std::regex recipient_name_pattern("^__recip_version1\\.0_#[0-9A-Fa-f]{8}$");
const std::regex attachment_name_pattern("^__attach_version1\\.0_#[0-9A-Fa-f]{8}$");

} // namespace

msg_reader::msg_reader(cfb::cfb_reader cfb_reader, const bool strict)
    : cfb_reader_(std::move(cfb_reader)),
      strict_(strict)
{
    validate_top_level();
}

msg_reader msg_reader::from_file(const std::filesystem::path& path, const bool strict)
{
    return msg_reader(cfb::cfb_reader::from_file(path), strict);
}

msg_reader msg_reader::from_stream(std::istream& stream, const bool strict)
{
    return msg_reader(cfb::cfb_reader::from_stream(stream), strict);
}

const cfb::cfb_reader& msg_reader::cfb() const noexcept
{
    return cfb_reader_;
}

bool msg_reader::strict() const noexcept
{
    return strict_;
}

const std::vector<std::string>& msg_reader::validation_issues() const noexcept
{
    return validation_issues_;
}

void msg_reader::validate_top_level()
{
    std::size_t named_mapping_count = 0U;
    std::size_t property_stream_count = 0U;

    for (const auto child_id : cfb_reader_.child_ids(cfb::cfb_constants::root_stream_id))
    {
        const auto& child = cfb_reader_.get_entry(child_id);
        if (child.is_storage() && child.name == msg_constants::named_property_mapping_storage_name)
        {
            ++named_mapping_count;
        }

        if (child.is_stream() && child.name == msg_constants::property_stream_name)
        {
            ++property_stream_count;
        }

        if (child.is_storage() && child.name.rfind(msg_constants::recipient_storage_prefix.data(), 0) == 0 &&
            !std::regex_match(child.name.cbegin(), child.name.cend(), recipient_name_pattern))
        {
            throw msg_exception("Invalid recipient storage naming pattern.");
        }

        if (child.is_storage() && child.name.rfind(msg_constants::attachment_storage_prefix.data(), 0) == 0 &&
            !std::regex_match(child.name.cbegin(), child.name.cend(), attachment_name_pattern))
        {
            throw msg_exception("Invalid attachment storage naming pattern.");
        }
    }

    if (named_mapping_count != 1U)
    {
        throw msg_exception("Top-level MSG storage must contain exactly one named property mapping storage.");
    }

    if (property_stream_count != 1U)
    {
        throw msg_exception("Top-level MSG storage must contain exactly one property stream.");
    }
}

} // namespace aspose::email::foss::msg
