#include "aspose/email/foss/msg/mapi_property.hpp"

#include <utility>

namespace aspose::email::foss::msg
{

mapi_property::mapi_property(const std::uint16_t property_id, const std::uint16_t property_type, std::any value, const std::uint32_t flags)
    : property_id_(property_id),
      property_type_(property_type),
      value_(std::move(value)),
      flags_(flags)
{
}

std::uint16_t mapi_property::property_id() const noexcept
{
    return property_id_;
}

std::uint16_t mapi_property::property_type() const noexcept
{
    return property_type_;
}

const std::any& mapi_property::value() const noexcept
{
    return value_;
}

std::any& mapi_property::value() noexcept
{
    return value_;
}

void mapi_property::set_value(std::any value)
{
    value_ = std::move(value);
}

std::uint32_t mapi_property::flags() const noexcept
{
    return flags_;
}

void mapi_property::set_flags(const std::uint32_t value) noexcept
{
    flags_ = value;
}

std::uint32_t mapi_property::property_tag() const noexcept
{
    return (static_cast<std::uint32_t>(property_id_) << 16U) | property_type_;
}

} // namespace aspose::email::foss::msg
