#include "mime_model.hpp"

namespace aspose::email::foss::msg::mime
{

bool mime_entity::is_multipart() const noexcept
{
    return content_type.rfind("multipart/", 0) == 0;
}

} // namespace aspose::email::foss::msg::mime
