#include "aspose/email/foss/cfb/directory_entry.hpp"
#include "aspose/email/foss/cfb/header.hpp"

namespace aspose::email::foss::cfb
{

std::size_t header::sector_size() const noexcept
{
    return std::size_t {1} << sector_shift;
}

std::size_t header::mini_sector_size() const noexcept
{
    return std::size_t {1} << mini_sector_shift;
}

bool directory_entry::is_storage() const noexcept
{
    return object_type == directory_object_type::storage_object || object_type == directory_object_type::root_storage_object;
}

bool directory_entry::is_stream() const noexcept
{
    return object_type == directory_object_type::stream_object;
}

bool directory_entry::is_root() const noexcept
{
    return object_type == directory_object_type::root_storage_object;
}

} // namespace aspose::email::foss::cfb
