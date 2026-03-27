#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace aspose::email::foss::msg
{

class msg_stream
{
public:
    explicit msg_stream(std::string name, std::vector<std::uint8_t> data = {});

    std::string name;
    std::vector<std::uint8_t> data;
    std::array<std::uint8_t, 16> clsid {};
    std::uint32_t state_bits {};
    std::uint64_t creation_time {};
    std::uint64_t modified_time {};
};

} // namespace aspose::email::foss::msg
