#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "aspose/email/foss/cfb/cfb_constants.hpp"
#include "aspose/email/foss/cfb/cfb_document.hpp"
#include "aspose/email/foss/cfb/cfb_reader.hpp"
#include "aspose/email/foss/cfb/cfb_storage.hpp"
#include "aspose/email/foss/cfb/cfb_stream.hpp"
#include "aspose/email/foss/cfb/cfb_writer.hpp"

namespace
{

int require(const bool condition, const char* message)
{
    if (condition)
    {
        return 0;
    }

    std::cerr << message << '\n';
    return 1;
}

std::vector<std::uint8_t> make_bytes(const std::size_t count, const std::uint8_t seed)
{
    std::vector<std::uint8_t> result(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        result[index] = static_cast<std::uint8_t>((seed + index) & 0xFFU);
    }

    return result;
}

} // namespace

int main()
{
    using namespace aspose::email::foss::cfb;

    cfb_document document;
    auto& root = document.root();
    auto& folder = root.add_storage(cfb_storage("Folder"));
    folder.add_stream(cfb_stream("__properties_version1.0", {1U, 2U, 3U, 4U}));
    root.add_stream(cfb_stream("MiniStream", make_bytes(128U, 0x10U)));
    root.add_stream(cfb_stream("LargeStream", make_bytes(5000U, 0x40U)));

    const auto bytes = cfb_writer::to_bytes(document);
    if (auto rc = require(!bytes.empty(), "Serialized CFB payload must not be empty"); rc != 0)
    {
        return rc;
    }

    auto reader = cfb_reader::from_bytes(bytes);
    if (auto rc = require(reader.header().major_version == 3U, "Unexpected CFB major version"); rc != 0)
    {
        return rc;
    }

    const auto folder_id = reader.resolve_path({"Folder"});
    if (auto rc = require(folder_id.has_value(), "Folder storage was not found"); rc != 0)
    {
        return rc;
    }

    const auto props_id = reader.resolve_path({"Folder", "__properties_version1.0"});
    if (auto rc = require(props_id.has_value(), "Nested property stream was not found"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(reader.get_stream_data(*props_id) == std::vector<std::uint8_t>({1U, 2U, 3U, 4U}), "Unexpected nested stream data"); rc != 0)
    {
        return rc;
    }

    const auto large_stream_id = reader.resolve_path({"LargeStream"});
    if (auto rc = require(large_stream_id.has_value(), "Large stream was not found"); rc != 0)
    {
        return rc;
    }

    if (auto rc = require(reader.get_stream_data(*large_stream_id) == make_bytes(5000U, 0x40U), "Large stream round-trip failed"); rc != 0)
    {
        return rc;
    }

    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    cfb_writer::write_stream(document, stream);
    stream.seekg(0, std::ios::beg);
    auto stream_reader = cfb_reader::from_stream(stream);
    if (auto rc = require(stream_reader.resolve_path({"MiniStream"}).has_value(), "Stream-based load failed"); rc != 0)
    {
        return rc;
    }

    const auto roundtrip_document = cfb_document::from_reader(reader);
    const auto roundtrip_bytes = cfb_writer::to_bytes(roundtrip_document);
    if (auto rc = require(bytes == roundtrip_bytes, "Deterministic CFB round-trip failed"); rc != 0)
    {
        return rc;
    }

    const auto temp_path = std::filesystem::current_path() / "cfb-roundtrip-test.cfb";
    cfb_writer::write_file(document, temp_path);
    auto file_reader = cfb_reader::from_file(temp_path);
    std::filesystem::remove(temp_path);
    if (auto rc = require(file_reader.resolve_path({"Folder", "__properties_version1.0"}).has_value(), "File-based load failed"); rc != 0)
    {
        return rc;
    }

    return EXIT_SUCCESS;
}
