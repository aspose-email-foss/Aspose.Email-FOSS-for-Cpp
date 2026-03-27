#pragma once

#include <cstdint>

namespace aspose::email::foss::cfb
{

enum class directory_object_type : std::uint8_t
{
    unknown_or_unallocated = 0,
    storage_object = 1,
    stream_object = 2,
    root_storage_object = 5
};

} // namespace aspose::email::foss::cfb
