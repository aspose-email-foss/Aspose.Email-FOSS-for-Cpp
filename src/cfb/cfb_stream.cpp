#include "aspose/email/foss/cfb/cfb_stream.hpp"

#include <utility>

namespace aspose::email::foss::cfb
{

cfb_stream::cfb_stream(std::string name, std::vector<std::uint8_t> data)
    : cfb_node(kind::stream, std::move(name)),
      data_(std::move(data))
{
}

const std::vector<std::uint8_t>& cfb_stream::data() const noexcept
{
    return data_;
}

std::vector<std::uint8_t>& cfb_stream::data() noexcept
{
    return data_;
}

void cfb_stream::set_data(std::vector<std::uint8_t> value)
{
    data_ = std::move(value);
}

std::unique_ptr<cfb_node> cfb_stream::clone_impl() const
{
    return std::make_unique<cfb_stream>(*this);
}

} // namespace aspose::email::foss::cfb
