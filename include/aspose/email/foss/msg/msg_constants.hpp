#pragma once

#include <cstdint>
#include <string_view>

namespace aspose::email::foss::msg
{

namespace msg_constants
{

inline constexpr std::string_view attachment_storage_prefix = "__attach_version1.0_#";
inline constexpr std::string_view embedded_message_storage_name = "__substg1.0_3701000D";
inline constexpr std::uint32_t max_attachment_storages = 2048u;
inline constexpr std::uint32_t max_recipient_storages = 2048u;
inline constexpr std::string_view named_property_mapping_storage_name = "__nameid_version1.0";
inline constexpr std::uint32_t propattr_mandatory = 1u;
inline constexpr std::uint32_t propattr_readable = 2u;
inline constexpr std::uint32_t propattr_writable = 4u;
inline constexpr std::string_view property_stream_name = "__properties_version1.0";
inline constexpr std::string_view property_value_stream_prefix = "__substg1.0_";
inline constexpr std::string_view recipient_storage_prefix = "__recip_version1.0_#";

} // namespace msg_constants

} // namespace aspose::email::foss::msg
