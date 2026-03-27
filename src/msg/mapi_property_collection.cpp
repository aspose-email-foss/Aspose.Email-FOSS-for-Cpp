#include "aspose/email/foss/msg/mapi_property_collection.hpp"

namespace aspose::email::foss::msg
{

mapi_property& mapi_property_collection::set(mapi_property property)
{
    const auto key = std::make_pair(property.property_id(), property.property_type());
    auto [iterator, inserted] = properties_.insert_or_assign(key, std::move(property));
    return iterator->second;
}

mapi_property& mapi_property_collection::add(const std::uint16_t property_id, const std::uint16_t property_type, std::any value, const std::uint32_t flags)
{
    return set(mapi_property(property_id, property_type, std::move(value), flags));
}

mapi_property* mapi_property_collection::get(const std::uint16_t property_id, const std::optional<std::uint16_t> property_type) noexcept
{
    if (property_type.has_value())
    {
        const auto iterator = properties_.find(std::make_pair(property_id, *property_type));
        return iterator == properties_.end() ? nullptr : &iterator->second;
    }

    for (auto& [key, value] : properties_)
    {
        if (key.first == property_id)
        {
            return &value;
        }
    }

    return nullptr;
}

const mapi_property* mapi_property_collection::get(const std::uint16_t property_id, const std::optional<std::uint16_t> property_type) const noexcept
{
    return const_cast<mapi_property_collection*>(this)->get(property_id, property_type);
}

void mapi_property_collection::remove(const std::uint16_t property_id, const std::optional<std::uint16_t> property_type)
{
    if (property_type.has_value())
    {
        properties_.erase(std::make_pair(property_id, *property_type));
        return;
    }

    for (auto iterator = properties_.begin(); iterator != properties_.end();)
    {
        if (iterator->first.first == property_id)
        {
            iterator = properties_.erase(iterator);
            continue;
        }

        ++iterator;
    }
}

const std::map<std::pair<std::uint16_t, std::uint16_t>, mapi_property>& mapi_property_collection::items() const noexcept
{
    return properties_;
}

} // namespace aspose::email::foss::msg
