#ifndef _BASE_NAME_HPP
#define _BASE_NAME_HPP

#include <initializer_list>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

#include "utils/stringUtils.hpp"

namespace base
{

class Name
{
public:
    constexpr static auto SEPARATOR_S = "/";
    constexpr static auto SEPARATOR_C = '/';
    constexpr static auto MAX_PARTS = 10;

private:
    std::vector<std::string> parts_;

    void assertSize(std::size_t size) const
    {
        if (0 == size)
        {
            throw std::runtime_error(
                fmt::format("Name cannot be empty.")
            );
        }
        if (MAX_PARTS < size)
        {
            throw std::runtime_error(
                fmt::format(
                    "Name must have at most {} parts; currently is {} parts.",
                    MAX_PARTS,
                    size
                )
            );
        }

        for (const auto& part : parts_)
        {
            if (part.empty())
            {
                throw std::runtime_error(fmt::format("Name cannot have empty parts."));
            }
        }
    }

public:
    Name() = default;
    ~Name() = default;

    Name(const std::vector<std::string>& parts)
        : parts_{parts}
    {
        assertSize(parts_.size());
    }

    Name(std::vector<std::string>&& parts)
        : parts_{std::move(parts)}
    {
        assertSize(parts_.size());
    }

    Name(std::string_view name)
        : parts_{base::utils::string::split(name, SEPARATOR_C)}
    {
        assertSize(parts_.size());
    }

    Name(const char* fullName)
        : Name{std::string_view{fullName}}
    {
    }

    Name(const Name& other) = default;
    Name(Name&& other) = default;
    Name& operator=(const Name& other) = default;
    Name& operator=(Name&& other) = default;

    friend bool operator==(const Name& rhs, const Name& lhs)
        { return rhs.parts_ == lhs.parts_; }

    friend bool operator!=(const Name& rhs, const Name& lhs)
        { return rhs.parts_ != lhs.parts_; }

    std::string toStr() const
    {
        return std::accumulate(
            parts_.cbegin() + 1,
            parts_.cend(),
            parts_.front(),
            [](const std::string& a, const std::string& b) -> std::string
            {
                return a + SEPARATOR_S + b;
            }
        );
    }

    operator std::string() const { return toStr(); }

    friend std::ostream& operator<<(std::ostream& os, const Name& name)
    {
        os << name.toStr();
        return os;
    }

    friend Name operator+(const Name& lhs, const Name& rhs)
    {
        auto parts = lhs.parts_;
        parts.insert(parts.end(), rhs.parts().begin(), rhs.parts().end());

        return Name(parts);
    }
    
    bool operator<(const Name& other) const
    {
        return std::lexicographical_compare(
            parts_.begin(),
            parts_.end(),
            other.parts_.begin(),
            other.parts_.end()
        );

    }

    const std::vector<std::string>& parts() const { return parts_; }
};

} // namespace base


// hash for Name
namespace std
{
template <>
struct hash<base::Name>
{
    std::size_t operator()(const base::Name& name) const
    {
        std::hash<std::string> hasher;
        std::size_t hashValue{0};

        for (const auto& part : name.parts())
        {
            hashValue ^= hasher(part);
        }

        return hashValue;
    }
};
} // namespace std

// Make Name formattable with fmt
template <>
struct fmt::formatter<base::Name> : formatter<std::string>
{
    template <typename FormatCtx>
    auto format(const base::Name& name, FormatCtx& ctx) const
    {
        return formatter<std::string>::format(name.toStr(), ctx);
    }
};

#endif // _BASE_NAME_HPP
