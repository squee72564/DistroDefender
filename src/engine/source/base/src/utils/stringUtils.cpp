#include "base/utils/stringUtils.hpp"

namespace base::utils::string
{

std::vector<std::string> split(std::string_view str, const char delim)
{
    std::vector<std::string> ret;

    if (!str.empty() && str[0] == delim)
    {
        str = str.substr(1);
    }

    auto pos = str.find(delim);

    while (pos != std::string_view::npos)
    {
        ret.emplace_back(str.substr(0, pos));

        str = str.substr(pos+1);

        pos = str.find(delim);
    }

    if (!str.empty())
    {
        ret.emplace_back(str);
    }

    return ret;
}

std::string join(const std::vector<std::string>& strVec,
                 std::string_view separator,
                 const bool startsWithSeparator)
{
    std::string res{};

    for (std::size_t i = 0; i < strVec.size(); ++i) {
        res.append((!startsWithSeparator && 0 == i) ? "" : separator);
        res.append(strVec.at(i));
    }

    return res;
}

bool startsWith(std::string_view str, std::string_view prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view str, std::string_view suffix)
{
    if (suffix.size() > str.size()) return false;

    return str.compare(str.size() - suffix.size() , suffix.size(), suffix) == 0;
}

std::vector<std::string> splitEscaped(std::string_view input,
                                      const char splitChar,
                                      const char escape)
{
    std::vector<std::string> ret;

    ret.emplace_back("");

    for (std::size_t i = 0; i < input.size(); ++i) {
        const char thisChar = input[i];

        if (thisChar == escape && i + 1 < input.size())
        {
            const char nextChar = input[i+1];

            if (nextChar == escape || nextChar == splitChar)
            {
                ret.back() += nextChar;
                ++i;
            }
            else
            {
                ret.back() += thisChar;
            }
        }
        else if (thisChar == splitChar)
        {
            ret.push_back("");
        }
        else
        {
            ret.back() += thisChar;
        }
    }

    return ret;
}

std::string unescapeString(std::string_view str, char escapeChar, std::string_view escapedChars, bool strictMode)
{
    std::string res{};
    res.reserve(str.size());

    for (std::size_t i = 0; i < str.size(); ++i) 
    {
        const char currChar = str[i];
        if (currChar != escapeChar || i + 1 == str.size())
        {
            res += currChar;
            continue;
        }

        const char nextChar = str[i+1];
        if (nextChar == escapeChar || escapedChars.find(nextChar) != std::string_view::npos)
        {
            res += nextChar;
            i++;
        }
        else
        {
            res += currChar;
        }
    }

    return res;
}

std::string toUpperCase(std::string_view str)
{
    std::string ret{str};
    std::transform(
        ret.begin(),
        ret.end(),
        ret.begin(),
        [](unsigned char c){ return std::toupper(c); }
    );
    return ret;
}

std::string toLowerCase(std::string_view str)
{
    std::string ret{str};
    std::transform(
        ret.begin(),
        ret.end(),
        ret.begin(),
        [](unsigned char c){ return std::tolower(c); }
    );
    return ret;
}

bool replaceFirst(std::string& data, const std::string& toSearch, const std::string& toReplace)
{
    auto pos{data.find(toSearch)};

    if (std::string::npos == pos) return false;

    data.replace(pos, toSearch.size(), toReplace);
    return true;
}

std::string leftTrim(const std::string& str, const std::string& args)
{
    const auto pos{str.find_first_not_of(args)};

    return (pos != std::string::npos) ? str.substr(pos) : "";
}

std::string rightTrim(const std::string& str, const std::string& args)
{
    const auto pos{str.find_last_not_of(args)};

    return (pos != std::string::npos) ? str.substr(0, pos+1) : "";
}

std::string trim(const std::string& str, const std::string& args)
{
    return leftTrim(rightTrim(str, args), args);
}

std::string toSentenceCase(const std::string& str)
{
    if (str.empty()) return "";

    std::string ret{toLowerCase(str)};
    ret[0] = static_cast<char>(std::toupper(ret[0]));

    return ret;
}

bool isNumber(const std::string& str)
{
    auto it{str.cbegin()};

    while (it != str.cend())
    {
        if (!std::isdigit(*it)) return false;
        it++;
    }

    return true;
}

bool replaceAll(std::string& data, const std::string_view toSearch, const std::string_view toReplace)
{
    if (toSearch.empty() || toSearch == toReplace || toReplace.find(toSearch) != std::string_view::npos)
    {
        return false;
    }

    size_t pos{data.find(toSearch)};

    if (pos == std::string::npos)
    {
        return false;
    }

    while (pos != std::string::npos)
    {
        data.replace(pos, toSearch.size(), toReplace);
        pos = data.find(toSearch, pos);
    }

    return true;
}

bool hasUpperCaseCharacters(const std::string& str)
{
    return std::any_of(
        str.cbegin(),
        str.cend(),
        [](unsigned char c) { return std::isupper(c);  }
    );
}

} // namespace base::utils::string
