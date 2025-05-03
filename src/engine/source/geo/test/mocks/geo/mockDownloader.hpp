
#ifndef _GEO_MOCK_DOWNLOADER_HPP
#define _GEO_MOCK_DOWNLOADER_HPP

#include <gmock/gmock.h>

#include <geo/idownloader.hpp>

namespace geo::mocks
{
class MockDownloader : public IDownloader
{
public:
    MOCK_METHOD((base::RespOrError<std::string>), downloadHTTPS, (std::string_view url), (const override));
    MOCK_METHOD(base::RespOrError<std::string>, computeMD5, (std::string_view data), (const override));
    MOCK_METHOD(base::RespOrError<std::string>, downloadMD5, (std::string_view url), (const override));
};
} // namespace geo::mocks
#endif // _GEO_MOCK_DOWNLOADER_HPP

