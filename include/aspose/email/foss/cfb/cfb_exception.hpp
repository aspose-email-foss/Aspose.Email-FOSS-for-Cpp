#pragma once

#include <stdexcept>

namespace aspose::email::foss::cfb
{

class cfb_exception : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

} // namespace aspose::email::foss::cfb
