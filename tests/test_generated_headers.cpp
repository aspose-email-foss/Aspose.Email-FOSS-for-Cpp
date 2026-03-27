#include <cstdlib>
#include <iostream>

#include "aspose/email/foss/version.hpp"
#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/cfb/directory_object_type.hpp"
#include "aspose/email/foss/msg/msg_constants.hpp"
#include "aspose/email/foss/msg/property_type_code.hpp"

namespace
{

int require(bool condition, const char* message)
{
    if (condition)
    {
        return 0;
    }

    std::cerr << message << '\n';
    return 1;
}

} // namespace

int main()
{
    if (auto rc = require(aspose::email::foss::version_string == "0.1.0", "Unexpected version string"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(aspose::email::foss::cfb::cfb_constants::nostream == 0xFFFFFFFFu, "Unexpected NOSTREAM value"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(static_cast<unsigned>(aspose::email::foss::cfb::directory_object_type::stream_object) == 2u, "Unexpected DirectoryObjectType::stream_object value"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(aspose::email::foss::msg::msg_constants::property_stream_name == "__properties_version1.0", "Unexpected PropertyStreamName value"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(static_cast<unsigned>(aspose::email::foss::msg::property_type_code::ptyp_string) == 31u, "Unexpected PropertyTypeCode::ptyp_string value"); rc != 0)
    {
        return rc;
    }

    return EXIT_SUCCESS;
}

