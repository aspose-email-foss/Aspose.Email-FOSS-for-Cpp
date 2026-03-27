#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <vector>

#include "aspose/email/foss/cfb/cfb_storage.hpp"

namespace aspose::email::foss::cfb
{

class cfb_reader;

class cfb_document
{
public:
    cfb_document();
    explicit cfb_document(
        cfb_storage root,
        std::uint16_t major_version = 3,
        std::uint16_t minor_version = 0x003E,
        std::uint32_t transaction_signature_number = 0);

    [[nodiscard]] cfb_storage& root() noexcept;
    [[nodiscard]] const cfb_storage& root() const noexcept;

    [[nodiscard]] std::uint16_t major_version() const noexcept;
    void set_major_version(std::uint16_t value) noexcept;

    [[nodiscard]] std::uint16_t minor_version() const noexcept;
    void set_minor_version(std::uint16_t value) noexcept;

    [[nodiscard]] std::uint32_t transaction_signature_number() const noexcept;
    void set_transaction_signature_number(std::uint32_t value) noexcept;

    [[nodiscard]] static cfb_document from_reader(const cfb_reader& reader);
    [[nodiscard]] static cfb_document from_file(const std::filesystem::path& path);
    [[nodiscard]] static cfb_document from_stream(std::istream& stream);
    [[nodiscard]] static cfb_document from_bytes(std::vector<std::uint8_t> data);
    [[nodiscard]] static cfb_document from_buffer(const std::uint8_t* data, std::size_t size);

private:
    cfb_storage root_;
    std::uint16_t major_version_;
    std::uint16_t minor_version_;
    std::uint32_t transaction_signature_number_;
};

} // namespace aspose::email::foss::cfb
