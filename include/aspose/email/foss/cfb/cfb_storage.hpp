#pragma once

#include <memory>
#include <string>
#include <vector>

#include "aspose/email/foss/cfb/cfb_node.hpp"

namespace aspose::email::foss::cfb
{

class cfb_stream;

class cfb_storage final : public cfb_node
{
public:
    explicit cfb_storage(std::string name);
    cfb_storage(const cfb_storage& other);
    cfb_storage(cfb_storage&&) noexcept = default;
    cfb_storage& operator=(const cfb_storage& other);
    cfb_storage& operator=(cfb_storage&&) noexcept = default;

    [[nodiscard]] const std::vector<std::unique_ptr<cfb_node>>& children() const noexcept;
    std::vector<std::unique_ptr<cfb_node>>& children() noexcept;

    cfb_storage& add_storage(cfb_storage storage);
    cfb_stream& add_stream(cfb_stream stream);

private:
    [[nodiscard]] std::unique_ptr<cfb_node> clone_impl() const override;

    std::vector<std::unique_ptr<cfb_node>> children_;
};

} // namespace aspose::email::foss::cfb
