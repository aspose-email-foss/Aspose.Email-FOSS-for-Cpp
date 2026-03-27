#pragma once

#include <cstdint>
#include <vector>

#include "mime_model.hpp"

namespace aspose::email::foss::msg::mime
{

class mime_writer
{
public:
    [[nodiscard]] std::vector<std::uint8_t> write(const mime_message_model& model) const;

private:
    [[nodiscard]] std::vector<std::uint8_t> write_entity(const mime_entity& entity) const;
};

class mime_boundary_factory
{
public:
    [[nodiscard]] static std::string create();
};

} // namespace aspose::email::foss::msg::mime
