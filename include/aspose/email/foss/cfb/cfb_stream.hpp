#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "aspose/email/foss/cfb/cfb_node.hpp"

namespace aspose::email::foss::cfb
{

class cfb_stream final : public cfb_node
{
public:
    explicit cfb_stream(std::string name, std::vector<std::uint8_t> data = {});

    [[nodiscard]] const std::vector<std::uint8_t>& data() const noexcept;
    std::vector<std::uint8_t>& data() noexcept;
    void set_data(std::vector<std::uint8_t> value);

private:
    [[nodiscard]] std::unique_ptr<cfb_node> clone_impl() const override;

    std::vector<std::uint8_t> data_;
};

} // namespace aspose::email::foss::cfb
