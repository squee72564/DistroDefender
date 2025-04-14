#ifndef _STRING_UTILS_HPP
#define _STRING_UTILS_HPP

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

namespace base::utils::string
{

std::vector<std::string> split(std::string_view str, const char delim);

using Delimiter = std::pair<char, bool>;

template <typename... Delim>
std::vector<std::string> splitMultiDelim(std::string_view str, Delim&&... delimiters)
{
    auto delimList = {delimiters...};

    std::vector<std::string> ret;

    std::size_t last = 0;
    for (std::size_t i = 0; i < str.size(); ++i)
    {
        for (auto delim : delimList)
        {
            const auto [delimChar, keepInOutput] = delim;

            if (str[i] == delimChar)
            {
                if (i > last)
                {
                    ret.emplace_back(str.substr(last, i-last));
                }

                if (keepInOutput)
                {
                    ret.emplace_back(std::string(1, delimChar));
                }

                last = i + 1;
                break;
            }
        }
    }

    if (last < str.size())
    {
        ret.emplace_back(str.substr(last));
    }

    return ret;
}

std::string join(const std::vector<std::string>& strVec,
                 std::string_view separator = "",
                 const bool startsWithSeparator = false);

bool startsWith(std::string_view str, std::string_view prefix);

bool endsWith(std::string_view str, std::string_view suffix);

std::vector<std::string> splitEscaped(std::string_view input,
                                      const char splitChar = '/',
                                      const char escape = '\\');

std::string unescapeString(std::string_view str, char escapeChar, std::string_view escapedChars, bool strictMode);

inline std::string unescapeString(std::string_view str, char escapeChar, const char escapedChar, bool strictMode)
{
    char buff[1] = {escapedChar};
    return unescapeString(str, escapeChar, std::string_view(buff, 1), strictMode);
}

std::string toUpperCase(std::string_view str);

std::string toLowerCase(std::string_view str);

bool replaceFirst(std::string& data, const std::string& toSearch, const std::string& toReplace);

std::string leftTrim(const std::string& str, const std::string& args = " ");

std::string rightTrim(const std::string& str, const std::string& args = " ");

std::string trim(const std::string& str, const std::string& args = " ");

std::string toSentenceCase(const std::string& str);

bool isNumber(const std::string& str);

bool replaceAll(std::string& data, const std::string_view toSearch, const std::string_view toReplace);

bool hasUpperCaseCharacters(const std::string& str);

} // namspace base::utils::string

#endif // _STRING_UTILS_HPP
