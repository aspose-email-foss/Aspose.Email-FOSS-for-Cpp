#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace aspose::email::foss::cfb
{

struct header
{
    std::array<std::uint8_t, 8> header_signature {};
    std::array<std::uint8_t, 16> header_clsid {};
    std::uint16_t minor_version {};
    std::uint16_t major_version {};
    std::uint16_t byte_order {};
    std::uint16_t sector_shift {};
    std::uint16_t mini_sector_shift {};
    std::array<std::uint8_t, 6> reserved {};
    std::uint32_t number_of_directory_sectors {};
    std::uint32_t number_of_fat_sectors {};
    std::uint32_t first_directory_sector_location {};
    std::uint32_t transaction_signature_number {};
    std::uint32_t mini_stream_cutoff_size {};
    std::uint32_t first_mini_fat_sector_location {};
    std::uint32_t number_of_mini_fat_sectors {};
    std::uint32_t first_difat_sector_location {};
    std::uint32_t number_of_difat_sectors {};
    std::vector<std::uint32_t> difat;

    [[nodiscard]] std::size_t sector_size() const noexcept;
    [[nodiscard]] std::size_t mini_sector_size() const noexcept;
};

} // namespace aspose::email::foss::cfb
