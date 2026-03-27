#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "aspose/email/foss/cfb/directory_entry.hpp"
#include "aspose/email/foss/cfb/header.hpp"

namespace aspose::email::foss::cfb
{

class cfb_reader
{
public:
    explicit cfb_reader(std::vector<std::uint8_t> data);

    [[nodiscard]] static cfb_reader from_file(const std::filesystem::path& path);
    [[nodiscard]] static cfb_reader from_stream(std::istream& stream);
    [[nodiscard]] static cfb_reader from_bytes(std::vector<std::uint8_t> data);
    [[nodiscard]] static cfb_reader from_buffer(const std::uint8_t* data, std::size_t size);

    [[nodiscard]] const header& header() const noexcept;
    [[nodiscard]] const std::vector<std::uint32_t>& difat() const noexcept;
    [[nodiscard]] const std::vector<std::uint32_t>& fat() const noexcept;
    [[nodiscard]] const std::vector<std::uint32_t>& mini_fat() const noexcept;
    [[nodiscard]] const std::vector<directory_entry>& directory_entries() const noexcept;
    [[nodiscard]] const directory_entry& root_entry() const;
    [[nodiscard]] std::size_t data_size() const noexcept;
    [[nodiscard]] std::size_t directory_entry_count() const noexcept;
    [[nodiscard]] std::size_t materialized_stream_count() const noexcept;

    [[nodiscard]] const directory_entry& get_entry(std::uint32_t stream_id) const;
    [[nodiscard]] const std::vector<std::uint8_t>& get_stream_data(std::uint32_t stream_id) const;

    [[nodiscard]] std::vector<std::uint32_t> storage_ids() const;
    [[nodiscard]] std::vector<std::uint32_t> stream_ids() const;
    [[nodiscard]] std::vector<std::uint32_t> child_ids(std::uint32_t storage_stream_id) const;
    [[nodiscard]] std::optional<std::uint32_t> find_child_by_name(std::uint32_t storage_stream_id, const std::string& name) const;
    [[nodiscard]] std::optional<std::uint32_t> resolve_path(const std::vector<std::string>& names, std::uint32_t start_stream_id = 0) const;

private:
    [[nodiscard]] cfb::header parse_header() const;
    void validate_header() const;
    [[nodiscard]] std::size_t sector_offset(std::uint32_t sector_number) const;
    [[nodiscard]] std::vector<std::uint8_t> read_sector(std::uint32_t sector_number) const;
    [[nodiscard]] std::vector<std::uint32_t> build_difat() const;
    [[nodiscard]] std::vector<std::uint32_t> build_fat() const;
    [[nodiscard]] std::vector<std::uint32_t> iter_chain(std::uint32_t start_sector) const;
    [[nodiscard]] std::vector<std::uint32_t> build_mini_fat() const;
    [[nodiscard]] std::vector<directory_entry> build_directory_entries() const;
    [[nodiscard]] std::size_t root_entry_index() const;
    [[nodiscard]] std::vector<std::uint8_t> read_stream_bytes(const directory_entry& entry) const;
    [[nodiscard]] std::vector<std::uint32_t> collect_siblings_in_order(std::uint32_t start_id) const;

    std::vector<std::uint8_t> data_;
    cfb::header header_;
    std::vector<std::uint32_t> difat_;
    std::vector<std::uint32_t> fat_;
    std::vector<std::uint32_t> mini_fat_;
    std::vector<directory_entry> directory_entries_;
    std::size_t root_entry_index_ {};
    std::vector<std::uint8_t> mini_stream_;
    mutable std::unordered_map<std::uint32_t, std::vector<std::uint8_t>> stream_data_;
};

} // namespace aspose::email::foss::cfb
