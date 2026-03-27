#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <utility>

#include "aspose/email/foss/msg/mapi_property.hpp"

namespace aspose::email::foss::msg
{

class mapi_property_collection
{
public:
    mapi_property& set(mapi_property property);
    mapi_property& add(std::uint16_t property_id, std::uint16_t property_type, std::any value = {}, std::uint32_t flags = 0U);
    [[nodiscard]] mapi_property* get(std::uint16_t property_id, std::optional<std::uint16_t> property_type = std::nullopt) noexcept;
    [[nodiscard]] const mapi_property* get(std::uint16_t property_id, std::optional<std::uint16_t> property_type = std::nullopt) const noexcept;
    void remove(std::uint16_t property_id, std::optional<std::uint16_t> property_type = std::nullopt);
    [[nodiscard]] const std::map<std::pair<std::uint16_t, std::uint16_t>, mapi_property>& items() const noexcept;

private:
    std::map<std::pair<std::uint16_t, std::uint16_t>, mapi_property> properties_;
};

} // namespace aspose::email::foss::msg
