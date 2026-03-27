#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "aspose/email/foss/msg/msg_storage_role.hpp"
#include "aspose/email/foss/msg/msg_stream.hpp"

namespace aspose::email::foss::msg
{

class msg_storage
{
public:
    explicit msg_storage(std::string name, msg_storage_role role = msg_storage_role::generic);

    msg_stream& add_stream(msg_stream stream);
    msg_storage& add_storage(msg_storage storage);

    [[nodiscard]] msg_stream* find_stream(const std::string& stream_name) noexcept;
    [[nodiscard]] const msg_stream* find_stream(const std::string& stream_name) const noexcept;
    [[nodiscard]] msg_storage* find_storage(const std::string& storage_name) noexcept;
    [[nodiscard]] const msg_storage* find_storage(const std::string& storage_name) const noexcept;

    std::string name;
    msg_storage_role role;
    std::array<std::uint8_t, 16> clsid {};
    std::uint32_t state_bits {};
    std::uint64_t creation_time {};
    std::uint64_t modified_time {};
    std::vector<msg_stream> streams;
    std::vector<msg_storage> storages;
};

} // namespace aspose::email::foss::msg
