#ifndef _BASE_DOT_PATH_HPP
#define _BASE_DOT_PATH_HPP

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
#include <string_view>

#include <fmt/core.h>
#include <fmt/format.h>

#include <base/utils/stringUtils.hpp>

class DotPath
{
private:
    std::string str_;
    std::vector<std::string> parts_;

    void parse()
    {
        parts_.clear();

        if (base::utils::string::startsWith(str_, "."))
        {
            str_ = str_.substr(1);
        }

        if (str_.empty())
        {
            return;
        }

        parts_ = base::utils::string::splitEscaped(str_, '.', '\\');

        for (const auto& part : parts_)
        {
            if (part.empty() && str_ != ".")
            {
                throw std::runtime_error("DotPath cannot have empty parts.");
            }
        }
    }

public:
    DotPath() = default;
    ~DotPath() = default;

    DotPath(const DotPath& other) = default;
    DotPath(DotPath&& other) noexcept = default;

    DotPath& operator=(const DotPath& other) = default;
    DotPath& operator=(DotPath&& other) noexcept = default;
    
    //DotPath(const std::string& str)
    //    : str_{str}
    //{
    //    parse();
    //}

    DotPath(std::string_view str)
        : str_{str}
    {
        parse();
    }

    DotPath(const decltype(parts_.cbegin())& begin, const decltype(parts_.cend())&  end)
        : str_{""}
    {
        for (auto it = begin; it != end; ++it)
        {
            str_ += *it;
            if (std::next(it) != end)
            {
                str_ += ".";
            }
        }
        
        parse();
    }

    auto cbegin() const { return parts_.cbegin(); }
    auto cend() const { return parts_.cend(); }

    friend bool operator==(const DotPath& lhs, const DotPath& rhs)
        { return lhs.str_ == rhs.str_; }
    friend bool operator!=(const DotPath& lhs, const DotPath& rhs)
        { return !(lhs == rhs); }

    friend  std::ostream& operator<<(std::ostream& os, const DotPath& dp)
    {
        os << dp.str_;
        return os;
    }

    explicit operator std::string() const { return str_; }

    const std::string& str() const { return str_; }

    const std::vector<std::string>& parts() const { return parts_; }

    bool isRoot() const { return parts_.empty(); }

    static DotPath fromJsonPath(const std::string& jsonPath)
    {
        if (jsonPath.empty())
        {
            return DotPath();
        }

        std::string path = (jsonPath[0] == '/') ? jsonPath.substr(1) : jsonPath;
        auto parts = base::utils::string::split(path, '/');

        std::transform(
            parts.begin(),
            parts.end(),
            parts.begin(),
            [](const std::string& part)
            {
                auto partCopy = part;

                std::size_t index = partCopy.find("~0", 0);;
                while (index != std::string::npos)
                {
                    partCopy.replace(index, 2, "~");
                    index = partCopy.find("~0", index+1);
                }

                index = partCopy.find("~1", 0);
                while (index != std::string::npos)
                {
                    partCopy.replace(index, 2, "/");
                    index  = partCopy.find("~1", index+1);
                }

                return partCopy;
            }
        );

        return DotPath(parts.cbegin(), parts.cend());
    }

    static DotPath append(const DotPath& rhs, const DotPath& lhs)
    {
        return DotPath(fmt::format("{}.{}", lhs.str(), rhs.str()));
    }
};

// Make DotPath hashable
namespace std
{
template<>
struct hash<DotPath> 
{
    std::size_t operator()(const DotPath& path) const
        { return std::hash<std::string>{}(path.str()); };
};
} // namespace std

// Make DotPath formattable by fmt
template<>
struct fmt::formatter<DotPath> : formatter<std::string>
{
    template <typename FormatCtx>
    auto format(const DotPath& path, FormatCtx& ctx)
    {
        return formatter<std::string>::format(path.str(), ctx);
    }
};

#endif // _BASE_DOT_PATH_HPP
