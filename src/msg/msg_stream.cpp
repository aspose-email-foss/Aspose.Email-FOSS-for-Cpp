#include "aspose/email/foss/msg/msg_stream.hpp"

#include <utility>

namespace aspose::email::foss::msg
{

msg_stream::msg_stream(std::string stream_name, std::vector<std::uint8_t> stream_data)
    : name(std::move(stream_name)),
      data(std::move(stream_data))
{
}

} // namespace aspose::email::foss::msg
