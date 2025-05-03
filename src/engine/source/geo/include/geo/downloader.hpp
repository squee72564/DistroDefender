#ifndef _GEO_DOWNLOADER_HPP
#define _GEO_DOWNLOADER_HPP

#include <geo/idownloader.hpp>

namespace geo
{

class Downloader : public IDownloader
{
public:
    Downloader() = default;
    virtual ~Downloader() = default;


    base::RespOrError<std::string> downloadHTTPS(std::string_view url) const override;
    base::RespOrError<std::string> computeMD5(std::string_view data) const override;
    base::RespOrError<std::string> downloadMD5(std::string_view url) const override;
};

} // namespace geo

#endif // _GEO_DOWNLOADER_HPP
