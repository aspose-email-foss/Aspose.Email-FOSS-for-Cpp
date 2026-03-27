#pragma once

#include <cstdint>

namespace aspose::email::foss::cfb
{

enum class sector_marker : std::uint32_t
{
    difsect = 4294967292,
    fatsect = 4294967293,
    endofchain = 4294967294,
    freesect = 4294967295
};

} // namespace aspose::email::foss::cfb
