#pragma once

#include <cstdint>
#include <filesystem>
#include <istream>

#include "aspose/email/foss/cfb/cfb_document.hpp"
#include "aspose/email/foss/msg/msg_reader.hpp"
#include "aspose/email/foss/msg/msg_storage.hpp"

namespace aspose::email::foss::msg
{

class msg_document
{
public:
    explicit msg_document(
        msg_storage root,
        std::uint16_t major_version = 3,
        std::uint16_t minor_version = 0x003E,
        std::uint32_t transaction_signature_number = 0,
        bool strict = false);

    [[nodiscard]] msg_storage& root() noexcept;
    [[nodiscard]] const msg_storage& root() const noexcept;
    [[nodiscard]] std::uint16_t major_version() const noexcept;
    [[nodiscard]] std::uint16_t minor_version() const noexcept;
    [[nodiscard]] std::uint32_t transaction_signature_number() const noexcept;
    [[nodiscard]] bool strict() const noexcept;

    [[nodiscard]] static msg_document from_reader(const msg_reader& reader);
    [[nodiscard]] static msg_document from_file(const std::filesystem::path& path, bool strict = false);
    [[nodiscard]] static msg_document from_stream(std::istream& stream, bool strict = false);

    [[nodiscard]] cfb::cfb_document to_cfb_document() const;

private:
    msg_storage root_;
    std::uint16_t major_version_;
    std::uint16_t minor_version_;
    std::uint32_t transaction_signature_number_;
    bool strict_;
};

} // namespace aspose::email::foss::msg
