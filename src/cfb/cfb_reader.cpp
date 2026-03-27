#include "aspose/email/foss/cfb/cfb_reader.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <type_traits>
#include <unordered_set>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/cfb/cfb_exception.hpp"
#include "aspose/email/foss/cfb/sector_marker.hpp"
#include "detail.hpp"

namespace aspose::email::foss::cfb
{

namespace
{

constexpr std::array<std::uint8_t, 8> file_signature {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};

template <typename Enum>
[[nodiscard]] constexpr auto to_underlying(const Enum value) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(value);
}

} // namespace

cfb_reader::cfb_reader(std::vector<std::uint8_t> data)
    : data_(std::move(data)),
      header_(parse_header())
{
    validate_header();
    difat_ = build_difat();
    fat_ = build_fat();
    mini_fat_ = build_mini_fat();
    directory_entries_ = build_directory_entries();
    root_entry_index_ = this->root_entry_index();
    mini_stream_ = root_entry().stream_size > 0U ? read_stream_bytes(root_entry()) : std::vector<std::uint8_t> {};
    stream_data_.emplace(root_entry().stream_id, mini_stream_);
}

cfb_reader cfb_reader::from_file(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream)
    {
        throw cfb_exception("Unable to open CFB file: " + path.string());
    }

    return from_stream(stream);
}

cfb_reader cfb_reader::from_stream(std::istream& stream)
{
    return cfb_reader(detail::read_all(stream));
}

cfb_reader cfb_reader::from_bytes(std::vector<std::uint8_t> data)
{
    return cfb_reader(std::move(data));
}

cfb_reader cfb_reader::from_buffer(const std::uint8_t* data, const std::size_t size)
{
    return cfb_reader(std::vector<std::uint8_t>(data, data + size));
}

const header& cfb_reader::header() const noexcept
{
    return header_;
}

const std::vector<std::uint32_t>& cfb_reader::difat() const noexcept
{
    return difat_;
}

const std::vector<std::uint32_t>& cfb_reader::fat() const noexcept
{
    return fat_;
}

const std::vector<std::uint32_t>& cfb_reader::mini_fat() const noexcept
{
    return mini_fat_;
}

const std::vector<directory_entry>& cfb_reader::directory_entries() const noexcept
{
    return directory_entries_;
}

const directory_entry& cfb_reader::root_entry() const
{
    return directory_entries_.at(root_entry_index_);
}

std::size_t cfb_reader::data_size() const noexcept
{
    return data_.size();
}

std::size_t cfb_reader::directory_entry_count() const noexcept
{
    return directory_entries_.size();
}

std::size_t cfb_reader::materialized_stream_count() const noexcept
{
    return stream_data_.size();
}

const directory_entry& cfb_reader::get_entry(const std::uint32_t stream_id) const
{
    if (stream_id >= directory_entries_.size())
    {
        throw cfb_exception("Invalid stream id.");
    }

    return directory_entries_[stream_id];
}

const std::vector<std::uint8_t>& cfb_reader::get_stream_data(const std::uint32_t stream_id) const
{
    const auto cached = stream_data_.find(stream_id);
    if (cached != stream_data_.end())
    {
        return cached->second;
    }

    const auto& entry = get_entry(stream_id);
    if (!entry.is_stream() && !entry.is_root())
    {
        throw cfb_exception("Stream data is not available for non-stream directory entries.");
    }

    return stream_data_.emplace(stream_id, read_stream_bytes(entry)).first->second;
}

std::vector<std::uint32_t> cfb_reader::storage_ids() const
{
    std::vector<std::uint32_t> result;
    for (const auto& entry : directory_entries_)
    {
        if (entry.is_storage())
        {
            result.push_back(entry.stream_id);
        }
    }

    return result;
}

std::vector<std::uint32_t> cfb_reader::stream_ids() const
{
    std::vector<std::uint32_t> result;
    for (const auto& entry : directory_entries_)
    {
        if (entry.is_stream())
        {
            result.push_back(entry.stream_id);
        }
    }

    return result;
}

std::vector<std::uint32_t> cfb_reader::child_ids(const std::uint32_t storage_stream_id) const
{
    const auto& storage = get_entry(storage_stream_id);
    if (!storage.is_storage() || storage.child_id == cfb_constants::nostream)
    {
        return {};
    }

    return collect_siblings_in_order(storage.child_id);
}

std::optional<std::uint32_t> cfb_reader::find_child_by_name(const std::uint32_t storage_stream_id, const std::string& name) const
{
    for (const auto child_id : child_ids(storage_stream_id))
    {
        if (get_entry(child_id).name == name)
        {
            return child_id;
        }
    }

    return std::nullopt;
}

std::optional<std::uint32_t> cfb_reader::resolve_path(const std::vector<std::string>& names, std::uint32_t start_stream_id) const
{
    auto current_id = start_stream_id;
    for (const auto& name : names)
    {
        const auto& current = get_entry(current_id);
        if (!current.is_storage())
        {
            return std::nullopt;
        }

        const auto child = find_child_by_name(current_id, name);
        if (!child.has_value())
        {
            return std::nullopt;
        }

        current_id = *child;
    }

    return current_id;
}

cfb::header cfb_reader::parse_header() const
{
    if (data_.size() < 512U)
    {
        throw cfb_exception("File is too small to contain a Compound File Binary header.");
    }

    if (!std::equal(file_signature.begin(), file_signature.end(), data_.begin()))
    {
        throw cfb_exception("Invalid Compound File Binary signature.");
    }

    cfb::header result;
    std::copy_n(data_.begin(), result.header_signature.size(), result.header_signature.begin());
    std::copy_n(data_.begin() + 8, result.header_clsid.size(), result.header_clsid.begin());
    result.minor_version = detail::read_u16(data_, 24);
    result.major_version = detail::read_u16(data_, 26);
    result.byte_order = detail::read_u16(data_, 28);
    result.sector_shift = detail::read_u16(data_, 30);
    result.mini_sector_shift = detail::read_u16(data_, 32);
    std::copy_n(data_.begin() + 34, result.reserved.size(), result.reserved.begin());
    result.number_of_directory_sectors = detail::read_u32(data_, 40);
    result.number_of_fat_sectors = detail::read_u32(data_, 44);
    result.first_directory_sector_location = detail::read_u32(data_, 48);
    result.transaction_signature_number = detail::read_u32(data_, 52);
    result.mini_stream_cutoff_size = detail::read_u32(data_, 56);
    result.first_mini_fat_sector_location = detail::read_u32(data_, 60);
    result.number_of_mini_fat_sectors = detail::read_u32(data_, 64);
    result.first_difat_sector_location = detail::read_u32(data_, 68);
    result.number_of_difat_sectors = detail::read_u32(data_, 72);
    result.difat.reserve(109U);
    for (std::size_t index = 0; index < 109U; ++index)
    {
        result.difat.push_back(detail::read_u32(data_, 76U + (index * 4U)));
    }

    if (result.byte_order != cfb_constants::byte_order_little_endian)
    {
        throw cfb_exception("Unsupported byte order; expected the little-endian marker.");
    }

    return result;
}

void cfb_reader::validate_header() const
{
    if (header_.major_version != 3U && header_.major_version != 4U)
    {
        throw cfb_exception("Unsupported Compound File Binary major version.");
    }

    const auto expected_sector_shift = header_.major_version == 3U ? 9U : 12U;
    if (header_.sector_shift != expected_sector_shift)
    {
        throw cfb_exception("Invalid sector shift for the declared Compound File Binary version.");
    }

    if (header_.mini_sector_shift != 6U)
    {
        throw cfb_exception("Invalid mini sector shift.");
    }

    if (header_.mini_stream_cutoff_size != cfb_constants::mini_stream_cutoff_size)
    {
        throw cfb_exception("Invalid mini stream cutoff size.");
    }

    if (std::any_of(header_.header_clsid.begin(), header_.header_clsid.end(), [](const auto value) { return value != 0U; }))
    {
        throw cfb_exception("Invalid header CLSID; expected all zero bytes.");
    }

    if (header_.major_version == 3U && header_.number_of_directory_sectors != 0U)
    {
        throw cfb_exception("Version 3 Compound File Binary headers must report zero directory sectors.");
    }
}

std::size_t cfb_reader::sector_offset(const std::uint32_t sector_number) const
{
    if (sector_number > cfb_constants::maxregsect)
    {
        throw cfb_exception("Invalid sector number.");
    }

    return static_cast<std::size_t>(sector_number + 1U) * header_.sector_size();
}

std::vector<std::uint8_t> cfb_reader::read_sector(const std::uint32_t sector_number) const
{
    const auto offset = sector_offset(sector_number);
    const auto end = offset + header_.sector_size();
    if (end > data_.size())
    {
        throw cfb_exception("Sector points past the end of the file.");
    }

    return std::vector<std::uint8_t>(
        data_.begin() + static_cast<std::ptrdiff_t>(offset),
        data_.begin() + static_cast<std::ptrdiff_t>(end));
}

std::vector<std::uint32_t> cfb_reader::build_difat() const
{
    std::vector<std::uint32_t> result;
    for (const auto entry : header_.difat)
    {
        if (entry != to_underlying(sector_marker::freesect))
        {
            result.push_back(entry);
        }
    }

    auto next_difat_sector = header_.first_difat_sector_location;
    std::unordered_set<std::uint32_t> visited;
    for (std::uint32_t index = 0; index < header_.number_of_difat_sectors; ++index)
    {
        if (next_difat_sector == to_underlying(sector_marker::endofchain) || next_difat_sector == to_underlying(sector_marker::freesect))
        {
            break;
        }

        if (!visited.insert(next_difat_sector).second)
        {
            throw cfb_exception("Cycle detected in DIFAT chain.");
        }

        const auto sector = read_sector(next_difat_sector);
        const auto entries_per_sector = (header_.sector_size() / 4U) - 1U;
        for (std::size_t entry_index = 0; entry_index < entries_per_sector; ++entry_index)
        {
            const auto value = detail::read_u32(sector, entry_index * 4U);
            if (value != to_underlying(sector_marker::freesect))
            {
                result.push_back(value);
            }
        }

        next_difat_sector = detail::read_u32(sector, header_.sector_size() - 4U);
    }

    if (result.size() > header_.number_of_fat_sectors)
    {
        result.resize(header_.number_of_fat_sectors);
    }

    return result;
}

std::vector<std::uint32_t> cfb_reader::build_fat() const
{
    std::vector<std::uint32_t> result;
    const auto entries_per_sector = header_.sector_size() / 4U;
    result.reserve(difat_.size() * entries_per_sector);
    for (const auto fat_sector : difat_)
    {
        const auto sector = read_sector(fat_sector);
        for (std::size_t index = 0; index < entries_per_sector; ++index)
        {
            result.push_back(detail::read_u32(sector, index * 4U));
        }
    }

    return result;
}

std::vector<std::uint32_t> cfb_reader::iter_chain(const std::uint32_t start_sector) const
{
    if (start_sector == to_underlying(sector_marker::endofchain) || start_sector == to_underlying(sector_marker::freesect))
    {
        return {};
    }

    std::vector<std::uint32_t> chain;
    std::unordered_set<std::uint32_t> seen;
    auto sector = start_sector;
    while (sector != to_underlying(sector_marker::endofchain))
    {
        if (!seen.insert(sector).second)
        {
            throw cfb_exception("Cycle detected in sector chain.");
        }

        if (sector >= fat_.size())
        {
            throw cfb_exception("Sector chain references a FAT entry outside the FAT range.");
        }

        chain.push_back(sector);
        const auto next = fat_[sector];
        if (next == to_underlying(sector_marker::fatsect) || next == to_underlying(sector_marker::difsect))
        {
            throw cfb_exception("Invalid sector chain reference to FAT or DIFAT markers.");
        }

        sector = next;
    }

    return chain;
}

std::vector<std::uint32_t> cfb_reader::build_mini_fat() const
{
    if (header_.number_of_mini_fat_sectors == 0U)
    {
        return {};
    }

    const auto chain = iter_chain(header_.first_mini_fat_sector_location);
    const auto entries_per_sector = header_.sector_size() / 4U;
    std::vector<std::uint32_t> result;
    result.reserve(chain.size() * entries_per_sector);
    for (const auto sector_number : chain)
    {
        const auto sector = read_sector(sector_number);
        for (std::size_t index = 0; index < entries_per_sector; ++index)
        {
            result.push_back(detail::read_u32(sector, index * 4U));
        }
    }

    return result;
}

std::vector<directory_entry> cfb_reader::build_directory_entries() const
{
    const auto chain = iter_chain(header_.first_directory_sector_location);
    std::vector<std::uint8_t> raw;
    raw.reserve(chain.size() * header_.sector_size());
    for (const auto sector_number : chain)
    {
        const auto sector = read_sector(sector_number);
        raw.insert(raw.end(), sector.begin(), sector.end());
    }

    std::vector<directory_entry> result;
    for (std::size_t offset = 0; offset + 128U <= raw.size(); offset += 128U)
    {
        const auto object_type_raw = raw[offset + 66U];
        const auto color_flag_raw = raw[offset + 67U];
        if (object_type_raw != to_underlying(directory_object_type::unknown_or_unallocated) &&
            object_type_raw != to_underlying(directory_object_type::storage_object) &&
            object_type_raw != to_underlying(directory_object_type::stream_object) &&
            object_type_raw != to_underlying(directory_object_type::root_storage_object))
        {
            throw cfb_exception("Invalid directory entry object_type value.");
        }

        if (color_flag_raw != to_underlying(directory_color_flag::red) &&
            color_flag_raw != to_underlying(directory_color_flag::black))
        {
            throw cfb_exception("Invalid directory entry color_flag value.");
        }

        directory_entry entry;
        entry.stream_id = static_cast<std::uint32_t>(result.size());
        entry.directory_entry_name_length = detail::read_u16(raw, offset + 64U);
        entry.object_type = static_cast<directory_object_type>(object_type_raw);
        entry.color_flag = static_cast<directory_color_flag>(color_flag_raw);
        entry.left_sibling_id = detail::read_u32(raw, offset + 68U);
        entry.right_sibling_id = detail::read_u32(raw, offset + 72U);
        entry.child_id = detail::read_u32(raw, offset + 76U);
        std::copy_n(raw.begin() + static_cast<std::ptrdiff_t>(offset + 80U), entry.clsid.size(), entry.clsid.begin());
        entry.state_bits = detail::read_u32(raw, offset + 96U);
        entry.creation_time = detail::read_u64(raw, offset + 100U);
        entry.modified_time = detail::read_u64(raw, offset + 108U);
        entry.starting_sector_location = detail::read_u32(raw, offset + 116U);
        entry.stream_size = detail::read_u64(raw, offset + 120U);
        if (header_.major_version == 3U)
        {
            entry.stream_size &= 0xFFFFFFFFULL;
        }

        if (entry.directory_entry_name_length >= 2U)
        {
            entry.name = detail::utf16le_to_utf8(raw.data() + offset, entry.directory_entry_name_length - 2U);
        }

        result.push_back(std::move(entry));
    }

    return result;
}

std::size_t cfb_reader::root_entry_index() const
{
    if (directory_entries_.empty())
    {
        throw cfb_exception("No directory entries found.");
    }

    const auto& root = directory_entries_.front();
    if (!root.is_root())
    {
        throw cfb_exception("Directory entry 0 is not the root storage.");
    }

    if (root.stream_id != cfb_constants::root_stream_id)
    {
        throw cfb_exception("Invalid root stream id.");
    }

    return 0U;
}

std::vector<std::uint8_t> cfb_reader::read_stream_bytes(const directory_entry& entry) const
{
    if (entry.stream_size == 0U)
    {
        return {};
    }

    if (entry.is_root() || entry.stream_size < header_.mini_stream_cutoff_size)
    {
        if (entry.is_root())
        {
            std::vector<std::uint8_t> payload;
            for (const auto sector_number : iter_chain(entry.starting_sector_location))
            {
                const auto sector = read_sector(sector_number);
                payload.insert(payload.end(), sector.begin(), sector.end());
            }

            if (payload.size() > entry.stream_size)
            {
                payload.resize(static_cast<std::size_t>(entry.stream_size));
            }

            return payload;
        }

        if (mini_stream_.empty())
        {
            return {};
        }

        std::vector<std::uint8_t> payload;
        std::unordered_set<std::uint32_t> seen;
        auto mini_sector = entry.starting_sector_location;
        while (mini_sector != to_underlying(sector_marker::endofchain))
        {
            if (!seen.insert(mini_sector).second)
            {
                throw cfb_exception("Cycle detected in mini FAT chain.");
            }

            if (mini_sector >= mini_fat_.size())
            {
                throw cfb_exception("Mini sector index is outside the mini FAT range.");
            }

            const auto offset = static_cast<std::size_t>(mini_sector) * header_.mini_sector_size();
            const auto end = offset + header_.mini_sector_size();
            if (end > mini_stream_.size())
            {
                throw cfb_exception("Mini sector points past the end of the mini stream.");
            }

            payload.insert(
                payload.end(),
                mini_stream_.begin() + static_cast<std::ptrdiff_t>(offset),
                mini_stream_.begin() + static_cast<std::ptrdiff_t>(end));
            mini_sector = mini_fat_[mini_sector];
        }

        if (payload.size() > entry.stream_size)
        {
            payload.resize(static_cast<std::size_t>(entry.stream_size));
        }

        return payload;
    }

    std::vector<std::uint8_t> payload;
    for (const auto sector_number : iter_chain(entry.starting_sector_location))
    {
        const auto sector = read_sector(sector_number);
        payload.insert(payload.end(), sector.begin(), sector.end());
    }

    if (payload.size() > entry.stream_size)
    {
        payload.resize(static_cast<std::size_t>(entry.stream_size));
    }

    return payload;
}

std::vector<std::uint32_t> cfb_reader::collect_siblings_in_order(const std::uint32_t start_id) const
{
    std::vector<std::uint32_t> result;
    std::vector<std::uint32_t> stack;
    std::unordered_set<std::uint32_t> active;
    std::unordered_set<std::uint32_t> emitted;
    auto current = start_id;

    while (current != cfb_constants::nostream || !stack.empty())
    {
        while (current != cfb_constants::nostream)
        {
            if (current >= directory_entries_.size())
            {
                throw cfb_exception("Invalid stream id in directory sibling tree.");
            }

            if (!active.insert(current).second)
            {
                throw cfb_exception("Cycle detected in directory sibling tree.");
            }

            stack.push_back(current);
            current = directory_entries_[current].left_sibling_id;
        }

        const auto node_id = stack.back();
        stack.pop_back();
        active.erase(node_id);
        if (!emitted.insert(node_id).second)
        {
            throw cfb_exception("Duplicate node encountered in directory sibling tree.");
        }

        result.push_back(node_id);
        current = directory_entries_[node_id].right_sibling_id;
    }

    return result;
}

} // namespace aspose::email::foss::cfb
