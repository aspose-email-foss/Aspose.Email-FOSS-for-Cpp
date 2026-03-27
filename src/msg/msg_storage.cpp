#include "aspose/email/foss/msg/msg_storage.hpp"

#include <utility>

namespace aspose::email::foss::msg
{

msg_storage::msg_storage(std::string storage_name, const msg_storage_role storage_role)
    : name(std::move(storage_name)),
      role(storage_role)
{
}

msg_stream& msg_storage::add_stream(msg_stream stream)
{
    streams.push_back(std::move(stream));
    return streams.back();
}

msg_storage& msg_storage::add_storage(msg_storage storage)
{
    storages.push_back(std::move(storage));
    return storages.back();
}

msg_stream* msg_storage::find_stream(const std::string& stream_name) noexcept
{
    for (auto& stream : streams)
    {
        if (stream.name == stream_name)
        {
            return &stream;
        }
    }

    return nullptr;
}

const msg_stream* msg_storage::find_stream(const std::string& stream_name) const noexcept
{
    for (const auto& stream : streams)
    {
        if (stream.name == stream_name)
        {
            return &stream;
        }
    }

    return nullptr;
}

msg_storage* msg_storage::find_storage(const std::string& storage_name) noexcept
{
    for (auto& storage : storages)
    {
        if (storage.name == storage_name)
        {
            return &storage;
        }
    }

    return nullptr;
}

const msg_storage* msg_storage::find_storage(const std::string& storage_name) const noexcept
{
    for (const auto& storage : storages)
    {
        if (storage.name == storage_name)
        {
            return &storage;
        }
    }

    return nullptr;
}

} // namespace aspose::email::foss::msg
