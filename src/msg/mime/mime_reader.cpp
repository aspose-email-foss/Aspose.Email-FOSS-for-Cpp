#include "mime_reader.hpp"

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

#include "../../cfb/detail.hpp"
#include "transfer_encoding.hpp"

namespace aspose::email::foss::msg::mime
{

namespace
{

[[nodiscard]] std::string lower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character)
    {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

[[nodiscard]] std::string trim(std::string value)
{
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t' || value.front() == '\r' || value.front() == '\n'))
    {
        value.erase(value.begin());
    }

    while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r' || value.back() == '\n'))
    {
        value.pop_back();
    }

    return value;
}

[[nodiscard]] std::vector<std::string> split_lines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        lines.push_back(line);
    }

    return lines;
}

[[nodiscard]] std::string header_value(const std::vector<mime_header>& headers, const std::string& name)
{
    const auto lower_name = lower(name);
    for (const auto& header : headers)
    {
        if (lower(header.name) == lower_name)
        {
            return header.value;
        }
    }

    return {};
}

[[nodiscard]] std::string parameter_value(const std::string& header, const std::string& parameter_name)
{
    const std::regex pattern(parameter_name + "=\\\"?([^\\\";]+)\\\"?", std::regex::icase);
    std::smatch match;
    return std::regex_search(header, match, pattern) ? match[1].str() : std::string {};
}

} // namespace

mime_message_model mime_reader::read(const std::vector<std::uint8_t>& data) const
{
    mime_message_model model;
    model.root = parse_entity(data);
    model.headers = model.root.headers;
    return model;
}

mime_message_model mime_reader::read(std::istream& stream) const
{
    return read(cfb::detail::read_all(stream));
}

mime_entity mime_reader::parse_entity(const std::vector<std::uint8_t>& data) const
{
    mime_entity entity;
    const auto text = std::string(data.begin(), data.end());
    const auto separator = text.find("\r\n\r\n") != std::string::npos ? text.find("\r\n\r\n") : text.find("\n\n");
    const auto header_text = separator == std::string::npos ? std::string {} : text.substr(0U, separator);
    const auto body_text = separator == std::string::npos ? text : text.substr(separator + (text.find("\r\n\r\n") != std::string::npos ? 4U : 2U));

    for (const auto& line : split_lines(header_text))
    {
        if (!line.empty() && (line.front() == ' ' || line.front() == '\t') && !entity.headers.empty())
        {
            entity.headers.back().value += " " + trim(line);
            continue;
        }

        const auto colon = line.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        entity.headers.push_back({trim(line.substr(0U, colon)), trim(line.substr(colon + 1U))});
    }

    const auto content_type_header = header_value(entity.headers, "Content-Type");
    if (!content_type_header.empty())
    {
        entity.content_type = lower(trim(content_type_header.substr(0U, content_type_header.find(';'))));
        const auto boundary = parameter_value(content_type_header, "boundary");
        if (!boundary.empty())
        {
            entity.boundary = boundary;
        }

        const auto charset = parameter_value(content_type_header, "charset");
        if (!charset.empty())
        {
            entity.charset = lower(charset);
        }
    }

    entity.content_transfer_encoding = lower(header_value(entity.headers, "Content-Transfer-Encoding"));
    entity.content_disposition = lower(header_value(entity.headers, "Content-Disposition"));
    entity.filename = parameter_value(header_value(entity.headers, "Content-Disposition"), "filename");
    if (entity.filename.empty())
    {
        entity.filename = parameter_value(content_type_header, "name");
    }

    entity.content_id = trim(header_value(entity.headers, "Content-ID"));

    if (!entity.boundary.empty() && entity.is_multipart())
    {
        const auto delimiter = std::string("--") + entity.boundary;
        const auto closing = delimiter + "--";
        auto start = std::size_t {0};
        while (true)
        {
            const auto boundary_pos = body_text.find(delimiter, start);
            if (boundary_pos == std::string::npos)
            {
                break;
            }

            const auto next_start = boundary_pos + delimiter.size();
            if (body_text.compare(boundary_pos, closing.size(), closing) == 0)
            {
                break;
            }

            const auto content_start = body_text.find_first_not_of("\r\n", next_start);
            if (content_start == std::string::npos)
            {
                break;
            }

            const auto next_boundary = body_text.find(delimiter, content_start);
            if (next_boundary == std::string::npos)
            {
                break;
            }

            entity.children.push_back(parse_entity(std::vector<std::uint8_t>(body_text.begin() + static_cast<std::ptrdiff_t>(content_start), body_text.begin() + static_cast<std::ptrdiff_t>(next_boundary))));
            start = next_boundary;
        }

        return entity;
    }

    entity.body = transfer_encoding_decoder::decode(entity.content_transfer_encoding, std::vector<std::uint8_t>(body_text.begin(), body_text.end()));
    return entity;
}

} // namespace aspose::email::foss::msg::mime
