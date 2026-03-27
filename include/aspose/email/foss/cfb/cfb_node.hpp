#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>

namespace aspose::email::foss::cfb
{

class cfb_node
{
public:
    enum class kind
    {
        storage,
        stream,
    };

    virtual ~cfb_node() = default;

    [[nodiscard]] kind node_kind() const noexcept;
    [[nodiscard]] bool is_storage() const noexcept;
    [[nodiscard]] bool is_stream() const noexcept;

    [[nodiscard]] const std::string& name() const noexcept;
    std::string& name() noexcept;

    [[nodiscard]] const std::array<std::uint8_t, 16>& clsid() const noexcept;
    std::array<std::uint8_t, 16>& clsid() noexcept;

    [[nodiscard]] std::uint32_t state_bits() const noexcept;
    void set_state_bits(std::uint32_t value) noexcept;

    [[nodiscard]] std::uint64_t creation_time() const noexcept;
    void set_creation_time(std::uint64_t value) noexcept;

    [[nodiscard]] std::uint64_t modified_time() const noexcept;
    void set_modified_time(std::uint64_t value) noexcept;

    [[nodiscard]] std::unique_ptr<cfb_node> clone() const;

protected:
    explicit cfb_node(kind node_kind, std::string name);
    cfb_node(const cfb_node&) = default;
    cfb_node(cfb_node&&) noexcept = default;
    cfb_node& operator=(const cfb_node&) = default;
    cfb_node& operator=(cfb_node&&) noexcept = default;

private:
    [[nodiscard]] virtual std::unique_ptr<cfb_node> clone_impl() const = 0;

    kind node_kind_;
    std::string name_;
    std::array<std::uint8_t, 16> clsid_ {};
    std::uint32_t state_bits_ {};
    std::uint64_t creation_time_ {};
    std::uint64_t modified_time_ {};
};

} // namespace aspose::email::foss::cfb
