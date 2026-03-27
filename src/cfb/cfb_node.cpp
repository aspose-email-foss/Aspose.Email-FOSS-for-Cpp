#include "aspose/email/foss/cfb/cfb_node.hpp"

#include <utility>

namespace aspose::email::foss::cfb
{

cfb_node::cfb_node(const kind node_kind, std::string name)
    : node_kind_(node_kind),
      name_(std::move(name))
{
}

cfb_node::kind cfb_node::node_kind() const noexcept
{
    return node_kind_;
}

bool cfb_node::is_storage() const noexcept
{
    return node_kind_ == kind::storage;
}

bool cfb_node::is_stream() const noexcept
{
    return node_kind_ == kind::stream;
}

const std::string& cfb_node::name() const noexcept
{
    return name_;
}

std::string& cfb_node::name() noexcept
{
    return name_;
}

const std::array<std::uint8_t, 16>& cfb_node::clsid() const noexcept
{
    return clsid_;
}

std::array<std::uint8_t, 16>& cfb_node::clsid() noexcept
{
    return clsid_;
}

std::uint32_t cfb_node::state_bits() const noexcept
{
    return state_bits_;
}

void cfb_node::set_state_bits(const std::uint32_t value) noexcept
{
    state_bits_ = value;
}

std::uint64_t cfb_node::creation_time() const noexcept
{
    return creation_time_;
}

void cfb_node::set_creation_time(const std::uint64_t value) noexcept
{
    creation_time_ = value;
}

std::uint64_t cfb_node::modified_time() const noexcept
{
    return modified_time_;
}

void cfb_node::set_modified_time(const std::uint64_t value) noexcept
{
    modified_time_ = value;
}

std::unique_ptr<cfb_node> cfb_node::clone() const
{
    return clone_impl();
}

} // namespace aspose::email::foss::cfb
