#ifndef _GEO_IDOWNLOADER_HPP
#define _GEO_IDOWNLOADER_HPP

#include <string_view>

#include <base/error.hpp>

namespace geo
{

class IDownloader
{
public:
    virtual ~IDownloader() = default;

    virtual base::RespOrError<std::string> downloadHTTPS(std::string_view url) const = 0;
    virtual base::RespOrError<std::string> computeMD5(std::string_view data) const = 0;
    virtual base::RespOrError<std::string> downloadMD5(std::string_view url) const = 0;
};

} // namespace geo

#endif // _GEO_IDOWNLOADER_HPP
