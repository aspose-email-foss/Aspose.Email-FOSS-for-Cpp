#pragma once

#include <cstdint>

namespace aspose::email::foss::msg
{

enum class property_type_code : std::uint16_t
{
    ptyp_integer16 = 2,
    ptyp_integer32 = 3,
    ptyp_floating32 = 4,
    ptyp_floating64 = 5,
    ptyp_currency = 6,
    ptyp_floating_time = 7,
    ptyp_error_code = 10,
    ptyp_boolean = 11,
    ptyp_object = 13,
    ptyp_integer64 = 20,
    ptyp_string8 = 30,
    ptyp_string = 31,
    ptyp_time = 64,
    ptyp_guid = 72,
    ptyp_binary = 258,
    ptyp_multiple_integer16 = 4098,
    ptyp_multiple_integer32 = 4099,
    ptyp_multiple_floating32 = 4100,
    ptyp_multiple_floating64 = 4101,
    ptyp_multiple_currency = 4102,
    ptyp_multiple_floating_time = 4103,
    ptyp_multiple_integer64 = 4116,
    ptyp_multiple_string8 = 4126,
    ptyp_multiple_string = 4127,
    ptyp_multiple_time = 4160,
    ptyp_multiple_guid = 4168,
    ptyp_multiple_binary = 4354
};

} // namespace aspose::email::foss::msg
