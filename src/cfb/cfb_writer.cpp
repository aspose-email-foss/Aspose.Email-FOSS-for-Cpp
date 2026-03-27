#include "aspose/email/foss/cfb/cfb_writer.hpp"

#include <algorithm>
#include <array>
#include <deque>
#include <fstream>
#include <type_traits>
#include <vector>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/cfb/cfb_document.hpp"
#include "aspose/email/foss/cfb/cfb_exception.hpp"
#include "aspose/email/foss/cfb/cfb_storage.hpp"
#include "aspose/email/foss/cfb/cfb_stream.hpp"
#include "aspose/email/foss/cfb/directory_color_flag.hpp"
#include "aspose/email/foss/cfb/directory_object_type.hpp"
#include "aspose/email/foss/cfb/sector_marker.hpp"
#include "detail.hpp"

namespace aspose::email::foss::cfb
{

namespace
{

template <typename Enum>
[[nodiscard]] constexpr auto to_underlying(const Enum value) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(value);
}

struct entry_record
{
    std::uint32_t stream_id {};
    std::string name;
    directory_object_type object_type {directory_object_type::unknown_or_unallocated};
    std::array<std::uint8_t, 16> clsid {};
    std::uint32_t state_bits {};
    std::uint64_t creation_time {};
    std::uint64_t modified_time {};
    std::vector<std::uint8_t> stream_data;
    std::vector<entry_record*> children;
    directory_color_flag color_flag {directory_color_flag::black};
    std::uint32_t left_sibling_id {cfb_constants::nostream};
    std::uint32_t right_sibling_id {cfb_constants::nostream};
    std::uint32_t child_id {cfb_constants::nostream};
    std::uint32_t starting_sector_location {cfb_constants::nostream};
    std::uint64_t stream_size {};

    [[nodiscard]] bool is_root() const noexcept
    {
        return object_type == directory_object_type::root_storage_object;
    }

    [[nodiscard]] bool is_storage() const noexcept
    {
        return object_type == directory_object_type::storage_object || object_type == directory_object_type::root_storage_object;
    }

    [[nodiscard]] bool is_stream() const noexcept
    {
        return object_type == directory_object_type::stream_object;
    }
};

struct chain
{
    std::vector<std::uint8_t> payload;
    std::vector<std::uint32_t> sectors;

    [[nodiscard]] std::uint32_t first_sector() const noexcept
    {
        return sectors.empty() ? to_underlying(sector_marker::endofchain) : sectors.front();
    }
};

class serializer
{
public:
    explicit serializer(const cfb_document& document)
        : document_(document),
          major_version_(document.major_version()),
          minor_version_(document.minor_version()),
          sector_size_(major_version_ == 3U ? 512U : 4096U),
          mini_sector_size_(64U),
          entries_per_fat_sector_(sector_size_ / 4U),
          entries_per_difat_sector_(entries_per_fat_sector_ - 1U),
          directory_entries_per_sector_(sector_size_ / 128U)
    {
        if (major_version_ != 3U && major_version_ != 4U)
        {
            throw cfb_exception("Unsupported CFB writer major version.");
        }
    }

    [[nodiscard]] std::vector<std::uint8_t> serialize()
    {
        if (document_.root().name() != cfb_constants::root_entry_name)
        {
            throw cfb_exception("Root storage name must be 'Root Entry'.");
        }

        auto records = build_entry_records(document_.root());
        assign_sibling_trees(records);

        const auto [mini_stream_payload, mini_fat_entries] = plan_mini_stream(records);
        chain root_chain {mini_stream_payload, {}};
        chain directory_chain {pack_directory_entries(records), {}};
        chain mini_fat_chain {pack_u32_array(mini_fat_entries), {}};

        std::vector<entry_record*> regular_stream_records;
        std::vector<chain> regular_stream_chains;
        for (auto& record : records)
        {
            if (record.is_stream() && record.stream_size >= cfb_constants::mini_stream_cutoff_size)
            {
                regular_stream_records.push_back(&record);
                regular_stream_chains.push_back(chain {record.stream_data, {}});
            }
        }

        std::uint32_t content_sector_count = sector_span_for_payload(root_chain.payload) +
            sector_span_for_payload(directory_chain.payload) +
            sector_span_for_payload(mini_fat_chain.payload);
        for (const auto& chain_value : regular_stream_chains)
        {
            content_sector_count += sector_span_for_payload(chain_value.payload);
        }

        const auto [fat_sector_count, difat_sector_count] = solve_fat_and_difat_sector_counts(content_sector_count);

        std::uint32_t next_sector = 0U;
        for (std::size_t index = 0; index < regular_stream_chains.size(); ++index)
        {
            allocate_chain_sectors(regular_stream_chains[index], next_sector);
            regular_stream_records[index]->starting_sector_location = regular_stream_chains[index].first_sector();
        }

        allocate_chain_sectors(root_chain, next_sector);
        allocate_chain_sectors(directory_chain, next_sector);
        allocate_chain_sectors(mini_fat_chain, next_sector);

        const auto fat_sector_numbers = allocate_sectors(next_sector, fat_sector_count);
        const auto difat_sector_numbers = allocate_sectors(next_sector, difat_sector_count);
        const auto total_sector_count = next_sector;

        auto& root_record = records.front();
        root_record.starting_sector_location = root_chain.first_sector();
        root_record.stream_size = root_chain.payload.size();
        if (root_record.stream_size == 0U)
        {
            root_record.starting_sector_location = to_underlying(sector_marker::endofchain);
        }

        if (major_version_ == 3U && root_record.stream_size > 0x80000000ULL)
        {
            throw cfb_exception("Version 3 CFB writer cannot emit mini streams larger than 2 GB.");
        }

        directory_chain.payload = pack_directory_entries(records);

        std::vector<std::uint32_t> fat_entries(fat_sector_count * entries_per_fat_sector_, to_underlying(sector_marker::freesect));
        for (const auto& chain_value : regular_stream_chains)
        {
            mark_sector_chain(fat_entries, chain_value.sectors);
        }

        mark_sector_chain(fat_entries, root_chain.sectors);
        mark_sector_chain(fat_entries, directory_chain.sectors);
        mark_sector_chain(fat_entries, mini_fat_chain.sectors);
        for (const auto sector : fat_sector_numbers)
        {
            fat_entries[sector] = to_underlying(sector_marker::fatsect);
        }

        for (const auto sector : difat_sector_numbers)
        {
            fat_entries[sector] = to_underlying(sector_marker::difsect);
        }

        const auto fat_payload = pack_u32_array(fat_entries);
        auto sector_payloads = std::vector<std::vector<std::uint8_t>>(total_sector_count, std::vector<std::uint8_t>(sector_size_, 0U));
        assign_payloads(sector_payloads, split_payload(root_chain.payload, root_chain.sectors), root_chain.sectors);
        assign_payloads(sector_payloads, split_payload(directory_chain.payload, directory_chain.sectors), directory_chain.sectors);
        assign_payloads(sector_payloads, split_payload(mini_fat_chain.payload, mini_fat_chain.sectors), mini_fat_chain.sectors);
        for (std::size_t index = 0; index < regular_stream_chains.size(); ++index)
        {
            assign_payloads(sector_payloads, split_payload(regular_stream_chains[index].payload, regular_stream_chains[index].sectors), regular_stream_chains[index].sectors);
        }

        assign_payloads(sector_payloads, split_payload(fat_payload, fat_sector_numbers), fat_sector_numbers);
        assign_payloads(sector_payloads, build_difat_sectors(fat_sector_numbers, difat_sector_numbers), difat_sector_numbers);

        std::vector<std::uint32_t> header_difat = fat_sector_numbers;
        header_difat.resize(109U, to_underlying(sector_marker::freesect));
        if (header_difat.size() > 109U)
        {
            header_difat.resize(109U);
        }

        auto output = build_header(
            major_version_ == 3U ? 0U : static_cast<std::uint32_t>(directory_chain.sectors.size()),
            fat_sector_count,
            directory_chain.first_sector(),
            mini_fat_chain.sectors.empty() ? to_underlying(sector_marker::endofchain) : mini_fat_chain.first_sector(),
            static_cast<std::uint32_t>(mini_fat_chain.sectors.size()),
            difat_sector_numbers.empty() ? to_underlying(sector_marker::endofchain) : difat_sector_numbers.front(),
            static_cast<std::uint32_t>(difat_sector_numbers.size()),
            header_difat);

        for (const auto& sector_payload : sector_payloads)
        {
            output.insert(output.end(), sector_payload.begin(), sector_payload.end());
        }

        return output;
    }

private:
    [[nodiscard]] std::uint32_t sector_span_for_payload(const std::vector<std::uint8_t>& payload) const noexcept
    {
        return payload.empty() ? 0U : static_cast<std::uint32_t>((payload.size() + sector_size_ - 1U) / sector_size_);
    }

    [[nodiscard]] std::uint32_t mini_sector_span_for_size(const std::size_t size) const noexcept
    {
        return size == 0U ? 0U : static_cast<std::uint32_t>((size + mini_sector_size_ - 1U) / mini_sector_size_);
    }

    [[nodiscard]] std::size_t round_up(const std::size_t value, const std::size_t alignment) const noexcept
    {
        return value == 0U ? 0U : ((value + alignment - 1U) / alignment) * alignment;
    }

    [[nodiscard]] std::deque<entry_record> build_entry_records(const cfb_storage& root) const
    {
        std::deque<entry_record> records;
        const auto add_storage = [this, &records](const auto& self, const cfb_storage& storage, const bool is_root) -> entry_record*
        {
            validate_name(storage.name());
            entry_record record;
            record.stream_id = static_cast<std::uint32_t>(records.size());
            record.name = storage.name();
            record.object_type = is_root ? directory_object_type::root_storage_object : directory_object_type::storage_object;
            record.clsid = storage.clsid();
            record.state_bits = storage.state_bits();
            record.creation_time = storage.creation_time();
            record.modified_time = storage.modified_time();
            records.push_back(record);
            auto* current = &records.back();

            std::vector<const cfb_node*> children;
            children.reserve(storage.children().size());
            for (const auto& child : storage.children())
            {
                children.push_back(child.get());
            }

            std::sort(children.begin(), children.end(), [](const auto* left, const auto* right)
            {
                return detail::compare_directory_entry_names(left->name(), right->name()) < 0;
            });

            for (std::size_t index = 1; index < children.size(); ++index)
            {
                if (detail::compare_directory_entry_names(children[index - 1]->name(), children[index]->name()) == 0)
                {
                    throw cfb_exception("Duplicate sibling name under CFB comparison rules.");
                }
            }

            for (const auto* child : children)
            {
                if (child->is_storage())
                {
                    current->children.push_back(self(self, static_cast<const cfb_storage&>(*child), false));
                    continue;
                }

                const auto& stream = static_cast<const cfb_stream&>(*child);
                validate_name(stream.name());
                entry_record stream_record;
                stream_record.stream_id = static_cast<std::uint32_t>(records.size());
                stream_record.name = stream.name();
                stream_record.object_type = directory_object_type::stream_object;
                stream_record.clsid = stream.clsid();
                stream_record.state_bits = stream.state_bits();
                stream_record.creation_time = stream.creation_time();
                stream_record.modified_time = stream.modified_time();
                stream_record.stream_data = stream.data();
                stream_record.stream_size = stream_record.stream_data.size();
                records.push_back(std::move(stream_record));
                current->children.push_back(&records.back());
            }

            return current;
        };

        add_storage(add_storage, root, true);
        return records;
    }

    static void assign_sibling_trees(std::deque<entry_record>& records)
    {
        for (auto& record : records)
        {
            record.color_flag = directory_color_flag::black;
            record.left_sibling_id = cfb_constants::nostream;
            record.right_sibling_id = cfb_constants::nostream;
            record.child_id = cfb_constants::nostream;
        }

        const auto build = [](const auto& self, std::vector<entry_record*> items) -> std::uint32_t
        {
            if (items.empty())
            {
                return cfb_constants::nostream;
            }

            const auto mid = items.size() / 2U;
            auto* record = items[mid];
            std::vector<entry_record*> left(items.begin(), items.begin() + static_cast<std::ptrdiff_t>(mid));
            std::vector<entry_record*> right(items.begin() + static_cast<std::ptrdiff_t>(mid + 1U), items.end());
            record->left_sibling_id = self(self, std::move(left));
            record->right_sibling_id = self(self, std::move(right));
            record->color_flag = directory_color_flag::black;
            return record->stream_id;
        };

        for (auto& record : records)
        {
            if (record.is_storage() && !record.children.empty())
            {
                record.child_id = build(build, record.children);
            }
        }
    }

    [[nodiscard]] std::pair<std::vector<std::uint8_t>, std::vector<std::uint32_t>> plan_mini_stream(std::deque<entry_record>& records) const
    {
        std::vector<std::uint8_t> mini_stream;
        std::vector<std::uint32_t> mini_fat_entries;
        std::uint32_t next_mini_sector = 0U;

        for (auto& record : records)
        {
            if (!record.is_stream())
            {
                continue;
            }

            if (record.stream_size == 0U)
            {
                record.starting_sector_location = to_underlying(sector_marker::endofchain);
                continue;
            }

            if (record.stream_size >= cfb_constants::mini_stream_cutoff_size)
            {
                continue;
            }

            const auto mini_sector_count = mini_sector_span_for_size(record.stream_size);
            record.starting_sector_location = next_mini_sector;
            for (std::uint32_t offset = 0; offset < mini_sector_count; ++offset)
            {
                const auto current = next_mini_sector + offset;
                const auto next_value = offset + 1U < mini_sector_count ? current + 1U : to_underlying(sector_marker::endofchain);
                mini_fat_entries.push_back(next_value);
            }

            const auto start = mini_stream.size();
            const auto padded_length = static_cast<std::size_t>(mini_sector_count) * mini_sector_size_;
            mini_stream.resize(start + padded_length, 0U);
            std::copy(record.stream_data.begin(), record.stream_data.end(), mini_stream.begin() + static_cast<std::ptrdiff_t>(start));
            next_mini_sector += mini_sector_count;
        }

        return {mini_stream, mini_fat_entries};
    }

    [[nodiscard]] std::vector<std::uint8_t> build_header(
        const std::uint32_t number_of_directory_sectors,
        const std::uint32_t number_of_fat_sectors,
        const std::uint32_t first_directory_sector_location,
        const std::uint32_t first_mini_fat_sector_location,
        const std::uint32_t number_of_mini_fat_sectors,
        const std::uint32_t first_difat_sector_location,
        const std::uint32_t number_of_difat_sectors,
        const std::vector<std::uint32_t>& difat) const
    {
        std::vector<std::uint8_t> result(sector_size_, 0U);
        constexpr std::array<std::uint8_t, 8> signature {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
        std::copy(signature.begin(), signature.end(), result.begin());
        detail::write_u16(result, 24U, minor_version_);
        detail::write_u16(result, 26U, major_version_);
        detail::write_u16(result, 28U, static_cast<std::uint16_t>(cfb_constants::byte_order_little_endian));
        detail::write_u16(result, 30U, static_cast<std::uint16_t>(major_version_ == 3U ? 9U : 12U));
        detail::write_u16(result, 32U, 6U);
        detail::write_u32(result, 40U, number_of_directory_sectors);
        detail::write_u32(result, 44U, number_of_fat_sectors);
        detail::write_u32(result, 48U, first_directory_sector_location);
        detail::write_u32(result, 52U, document_.transaction_signature_number());
        detail::write_u32(result, 56U, cfb_constants::mini_stream_cutoff_size);
        detail::write_u32(result, 60U, first_mini_fat_sector_location);
        detail::write_u32(result, 64U, number_of_mini_fat_sectors);
        detail::write_u32(result, 68U, first_difat_sector_location);
        detail::write_u32(result, 72U, number_of_difat_sectors);
        for (std::size_t index = 0; index < difat.size(); ++index)
        {
            detail::write_u32(result, 76U + (index * 4U), difat[index]);
        }

        return result;
    }

    [[nodiscard]] std::vector<std::uint8_t> pack_directory_entries(const std::deque<entry_record>& records) const
    {
        const auto total_slots = round_up(records.size(), directory_entries_per_sector_);
        std::vector<std::uint8_t> payload(total_slots * 128U, 0U);
        for (std::size_t index = 0; index < records.size(); ++index)
        {
            const auto entry = pack_directory_entry(records[index]);
            std::copy(entry.begin(), entry.end(), payload.begin() + static_cast<std::ptrdiff_t>(index * 128U));
        }

        for (std::size_t index = records.size(); index < total_slots; ++index)
        {
            const auto entry = pack_unallocated_directory_entry();
            std::copy(entry.begin(), entry.end(), payload.begin() + static_cast<std::ptrdiff_t>(index * 128U));
        }

        return payload;
    }

    [[nodiscard]] std::vector<std::uint8_t> pack_directory_entry(const entry_record& record) const
    {
        std::vector<std::uint8_t> result(128U, 0U);
        auto name_bytes = detail::utf8_to_utf16le(record.name);
        name_bytes.push_back(0U);
        name_bytes.push_back(0U);
        if (name_bytes.size() > 64U)
        {
            throw cfb_exception("Directory entry name exceeds the 64-byte field.");
        }

        std::copy(name_bytes.begin(), name_bytes.end(), result.begin());
        detail::write_u16(result, 64U, static_cast<std::uint16_t>(name_bytes.size()));
        result[66] = to_underlying(record.object_type);
        result[67] = to_underlying(record.color_flag);
        detail::write_u32(result, 68U, record.left_sibling_id);
        detail::write_u32(result, 72U, record.right_sibling_id);
        detail::write_u32(result, 76U, record.child_id);
        std::copy(record.clsid.begin(), record.clsid.end(), result.begin() + 80);
        detail::write_u32(result, 96U, record.state_bits);
        detail::write_u64(result, 100U, record.creation_time);
        detail::write_u64(result, 108U, record.modified_time);

        if (record.object_type == directory_object_type::storage_object)
        {
            detail::write_u32(result, 116U, 0U);
            detail::write_u64(result, 120U, 0U);
            return result;
        }

        auto starting_sector = record.starting_sector_location;
        if (record.stream_size == 0U && record.is_stream())
        {
            starting_sector = to_underlying(sector_marker::endofchain);
        }

        if (major_version_ == 3U && record.stream_size > 0x80000000ULL)
        {
            throw cfb_exception("Version 3 CFB writer cannot emit streams larger than 2 GB.");
        }

        detail::write_u32(result, 116U, starting_sector);
        detail::write_u64(result, 120U, record.stream_size);
        return result;
    }

    [[nodiscard]] static std::vector<std::uint8_t> pack_unallocated_directory_entry()
    {
        std::vector<std::uint8_t> result(128U, 0U);
        detail::write_u32(result, 68U, cfb_constants::nostream);
        detail::write_u32(result, 72U, cfb_constants::nostream);
        detail::write_u32(result, 76U, cfb_constants::nostream);
        return result;
    }

    [[nodiscard]] std::vector<std::vector<std::uint8_t>> build_difat_sectors(
        const std::vector<std::uint32_t>& fat_sector_numbers,
        const std::vector<std::uint32_t>& difat_sector_numbers) const
    {
        std::vector<std::vector<std::uint8_t>> result;
        result.reserve(difat_sector_numbers.size());
        const auto remaining_start = std::min<std::size_t>(109U, fat_sector_numbers.size());
        const auto remaining_entries = std::vector<std::uint32_t>(fat_sector_numbers.begin() + static_cast<std::ptrdiff_t>(remaining_start), fat_sector_numbers.end());

        for (std::size_t index = 0; index < difat_sector_numbers.size(); ++index)
        {
            std::vector<std::uint8_t> payload(sector_size_, 0U);
            const auto start = index * entries_per_difat_sector_;
            for (std::size_t entry_index = 0; entry_index < entries_per_difat_sector_; ++entry_index)
            {
                const auto source_index = start + entry_index;
                const auto value = source_index < remaining_entries.size()
                    ? remaining_entries[source_index]
                    : to_underlying(sector_marker::freesect);
                detail::write_u32(payload, entry_index * 4U, value);
            }

            const auto next_sector = index + 1U < difat_sector_numbers.size()
                ? difat_sector_numbers[index + 1U]
                : to_underlying(sector_marker::endofchain);
            detail::write_u32(payload, sector_size_ - 4U, next_sector);
            result.push_back(std::move(payload));
        }

        return result;
    }

    [[nodiscard]] std::vector<std::vector<std::uint8_t>> split_payload(
        const std::vector<std::uint8_t>& payload,
        const std::vector<std::uint32_t>& sectors) const
    {
        std::vector<std::vector<std::uint8_t>> result;
        result.reserve(sectors.size());
        for (std::size_t index = 0; index < sectors.size(); ++index)
        {
            std::vector<std::uint8_t> chunk(sector_size_, 0U);
            const auto start = index * sector_size_;
            const auto copy_count = start < payload.size() ? std::min(sector_size_, payload.size() - start) : 0U;
            if (copy_count > 0U)
            {
                std::copy_n(payload.begin() + static_cast<std::ptrdiff_t>(start), copy_count, chunk.begin());
            }

            result.push_back(std::move(chunk));
        }

        return result;
    }

    [[nodiscard]] std::vector<std::uint8_t> pack_u32_array(const std::vector<std::uint32_t>& values) const
    {
        if (values.empty())
        {
            return {};
        }

        const auto padded_length = round_up(values.size(), entries_per_fat_sector_);
        std::vector<std::uint8_t> result(padded_length * 4U, 0U);
        for (std::size_t index = 0; index < padded_length; ++index)
        {
            const auto value = index < values.size() ? values[index] : to_underlying(sector_marker::freesect);
            detail::write_u32(result, index * 4U, value);
        }

        return result;
    }

    static void mark_sector_chain(std::vector<std::uint32_t>& fat_entries, const std::vector<std::uint32_t>& sectors)
    {
        if (sectors.empty())
        {
            return;
        }

        for (std::size_t index = 0; index + 1U < sectors.size(); ++index)
        {
            fat_entries[sectors[index]] = sectors[index + 1U];
        }

        fat_entries[sectors.back()] = to_underlying(sector_marker::endofchain);
    }

    [[nodiscard]] std::pair<std::uint32_t, std::uint32_t> solve_fat_and_difat_sector_counts(const std::uint32_t content_sector_count) const noexcept
    {
        std::uint32_t fat_sector_count = 0U;
        std::uint32_t difat_sector_count = 0U;
        while (true)
        {
            const auto total_sector_count = content_sector_count + fat_sector_count + difat_sector_count;
            const auto new_fat_sector_count = total_sector_count == 0U
                ? 0U
                : static_cast<std::uint32_t>((total_sector_count + entries_per_fat_sector_ - 1U) / entries_per_fat_sector_);
            const auto new_difat_sector_count = new_fat_sector_count <= 109U
                ? 0U
                : static_cast<std::uint32_t>((new_fat_sector_count - 109U + entries_per_difat_sector_ - 1U) / entries_per_difat_sector_);
            if (new_fat_sector_count == fat_sector_count && new_difat_sector_count == difat_sector_count)
            {
                return {fat_sector_count, difat_sector_count};
            }

            fat_sector_count = new_fat_sector_count;
            difat_sector_count = new_difat_sector_count;
        }
    }

    void validate_name(const std::string& name) const
    {
        if (name.empty())
        {
            throw cfb_exception("CFB writer does not support empty directory entry names.");
        }

        if (name.find('\0') != std::string::npos)
        {
            throw cfb_exception("CFB directory entry names must not contain embedded null characters.");
        }

        if (detail::utf16le_byte_count(name) + 2U > 64U)
        {
            throw cfb_exception("CFB directory entry names must fit into the 64-byte field.");
        }
    }

    void allocate_chain_sectors(chain& chain_value, std::uint32_t& next_sector) const
    {
        const auto sector_count = sector_span_for_payload(chain_value.payload);
        for (std::uint32_t index = 0; index < sector_count; ++index)
        {
            chain_value.sectors.push_back(next_sector++);
        }
    }

    [[nodiscard]] static std::vector<std::uint32_t> allocate_sectors(std::uint32_t& next_sector, const std::uint32_t count)
    {
        std::vector<std::uint32_t> sectors;
        sectors.reserve(count);
        for (std::uint32_t index = 0; index < count; ++index)
        {
            sectors.push_back(next_sector++);
        }

        return sectors;
    }

    void assign_payloads(
        std::vector<std::vector<std::uint8_t>>& destination,
        const std::vector<std::vector<std::uint8_t>>& source,
        const std::vector<std::uint32_t>& sector_numbers) const
    {
        for (std::size_t index = 0; index < source.size(); ++index)
        {
            destination[sector_numbers[index]] = source[index];
        }
    }

    const cfb_document& document_;
    std::uint16_t major_version_;
    std::uint16_t minor_version_;
    std::size_t sector_size_;
    std::size_t mini_sector_size_;
    std::size_t entries_per_fat_sector_;
    std::size_t entries_per_difat_sector_;
    std::size_t directory_entries_per_sector_;
};

} // namespace

std::vector<std::uint8_t> cfb_writer::to_bytes(const cfb_document& document)
{
    return serializer(document).serialize();
}

void cfb_writer::write_file(const cfb_document& document, const std::filesystem::path& path)
{
    std::ofstream stream(path, std::ios::binary);
    if (!stream)
    {
        throw cfb_exception("Unable to open output path for writing: " + path.string());
    }

    write_stream(document, stream);
}

void cfb_writer::write_stream(const cfb_document& document, std::ostream& stream)
{
    const auto bytes = to_bytes(document);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!stream)
    {
        throw cfb_exception("Failed to write serialized CFB data to output stream.");
    }
}

} // namespace aspose::email::foss::cfb
