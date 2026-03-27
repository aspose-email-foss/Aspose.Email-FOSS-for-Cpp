#pragma once

#include <istream>
#include <vector>

#include "mime_model.hpp"

namespace aspose::email::foss::msg::mime
{

class mime_reader
{
public:
    [[nodiscard]] mime_message_model read(const std::vector<std::uint8_t>& data) const;
    [[nodiscard]] mime_message_model read(std::istream& stream) const;

private:
    [[nodiscard]] mime_entity parse_entity(const std::vector<std::uint8_t>& data) const;
};

} // namespace aspose::email::foss::msg::mime
