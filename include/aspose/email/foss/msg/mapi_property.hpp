#pragma once

#include <any>
#include <cstdint>

namespace aspose::email::foss::msg
{

class mapi_property
{
public:
    mapi_property(std::uint16_t property_id, std::uint16_t property_type, std::any value = {}, std::uint32_t flags = 0U);

    [[nodiscard]] std::uint16_t property_id() const noexcept;
    [[nodiscard]] std::uint16_t property_type() const noexcept;
    [[nodiscard]] const std::any& value() const noexcept;
    std::any& value() noexcept;
    void set_value(std::any value);
    [[nodiscard]] std::uint32_t flags() const noexcept;
    void set_flags(std::uint32_t value) noexcept;
    [[nodiscard]] std::uint32_t property_tag() const noexcept;

private:
    std::uint16_t property_id_;
    std::uint16_t property_type_;
    std::any value_;
    std::uint32_t flags_;
};

} // namespace aspose::email::foss::msg
