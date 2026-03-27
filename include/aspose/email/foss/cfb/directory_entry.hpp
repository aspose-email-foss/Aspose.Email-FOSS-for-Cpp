#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "aspose/email/foss/cfb/directory_color_flag.hpp"
#include "aspose/email/foss/cfb/directory_object_type.hpp"

namespace aspose::email::foss::cfb
{

struct directory_entry
{
    std::uint32_t stream_id {};
    std::string name;
    std::uint16_t directory_entry_name_length {};
    directory_object_type object_type {directory_object_type::unknown_or_unallocated};
    directory_color_flag color_flag {directory_color_flag::black};
    std::uint32_t left_sibling_id {};
    std::uint32_t right_sibling_id {};
    std::uint32_t child_id {};
    std::array<std::uint8_t, 16> clsid {};
    std::uint32_t state_bits {};
    std::uint64_t creation_time {};
    std::uint64_t modified_time {};
    std::uint32_t starting_sector_location {};
    std::uint64_t stream_size {};

    [[nodiscard]] bool is_storage() const noexcept;
    [[nodiscard]] bool is_stream() const noexcept;
    [[nodiscard]] bool is_root() const noexcept;
};

} // namespace aspose::email::foss::cfb
