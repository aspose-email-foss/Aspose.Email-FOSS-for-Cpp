#pragma once

#include <filesystem>
#include <istream>
#include <string>
#include <vector>

#include "aspose/email/foss/cfb/cfb_reader.hpp"

namespace aspose::email::foss::msg
{

class msg_reader
{
public:
    explicit msg_reader(cfb::cfb_reader cfb_reader, bool strict = false);

    [[nodiscard]] static msg_reader from_file(const std::filesystem::path& path, bool strict = false);
    [[nodiscard]] static msg_reader from_stream(std::istream& stream, bool strict = false);

    [[nodiscard]] const cfb::cfb_reader& cfb() const noexcept;
    [[nodiscard]] bool strict() const noexcept;
    [[nodiscard]] const std::vector<std::string>& validation_issues() const noexcept;

private:
    void validate_top_level();

    cfb::cfb_reader cfb_reader_;
    bool strict_;
    std::vector<std::string> validation_issues_;
};

} // namespace aspose::email::foss::msg
