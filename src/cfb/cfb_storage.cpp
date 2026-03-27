#include "aspose/email/foss/cfb/cfb_storage.hpp"

#include <utility>

#include "aspose/email/foss/cfb/cfb_stream.hpp"

namespace aspose::email::foss::cfb
{

cfb_storage::cfb_storage(std::string name)
    : cfb_node(kind::storage, std::move(name))
{
}

cfb_storage::cfb_storage(const cfb_storage& other)
    : cfb_node(other)
{
    children_.reserve(other.children_.size());
    for (const auto& child : other.children_)
    {
        children_.push_back(child->clone());
    }
}

cfb_storage& cfb_storage::operator=(const cfb_storage& other)
{
    if (this == &other)
    {
        return *this;
    }

    cfb_node::operator=(other);
    children_.clear();
    children_.reserve(other.children_.size());
    for (const auto& child : other.children_)
    {
        children_.push_back(child->clone());
    }

    return *this;
}

const std::vector<std::unique_ptr<cfb_node>>& cfb_storage::children() const noexcept
{
    return children_;
}

std::vector<std::unique_ptr<cfb_node>>& cfb_storage::children() noexcept
{
    return children_;
}

cfb_storage& cfb_storage::add_storage(cfb_storage storage)
{
    auto child = std::make_unique<cfb_storage>(std::move(storage));
    auto& reference = *child;
    children_.push_back(std::move(child));
    return reference;
}

cfb_stream& cfb_storage::add_stream(cfb_stream stream)
{
    auto child = std::make_unique<cfb_stream>(std::move(stream));
    auto& reference = *child;
    children_.push_back(std::move(child));
    return reference;
}

std::unique_ptr<cfb_node> cfb_storage::clone_impl() const
{
    return std::make_unique<cfb_storage>(*this);
}

} // namespace aspose::email::foss::cfb
