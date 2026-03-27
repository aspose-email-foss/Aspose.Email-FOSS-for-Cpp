#pragma once

#include <cstdint>
#include <string_view>

namespace aspose::email::foss::cfb
{

namespace cfb_constants
{

inline constexpr std::uint32_t byte_order_little_endian = 65534u;
inline constexpr std::uint32_t maxregsect = 4294967290u;
inline constexpr std::uint32_t maxregsid = 4294967290u;
inline constexpr std::uint32_t mini_stream_cutoff_size = 4096u;
inline constexpr std::uint32_t nostream = 4294967295u;
inline constexpr std::string_view root_entry_name = "Root Entry";
inline constexpr std::uint32_t root_stream_id = 0u;

} // namespace cfb_constants

} // namespace aspose::email::foss::cfb
