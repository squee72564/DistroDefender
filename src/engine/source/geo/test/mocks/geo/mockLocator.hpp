#ifndef _GEO_MOCK_LOCATOR_HPP
#define _GEO_MOCK_LOCATOR_HPP

#include <gmock/gmock.h>

#include <geo/ilocator.hpp>

namespace geo::mocks
{
class MockLocator : public ILocator
{
public:
    MOCK_METHOD(base::RespOrError<std::string>, getString, (std::string_view ip, const DotPath& path), (override));
    MOCK_METHOD(base::RespOrError<uint32_t>, getUint32, (std::string_view ip, const DotPath& path), (override));
    MOCK_METHOD(base::RespOrError<double>, getDouble, (std::string_view ip, const DotPath& path), (override));
    MOCK_METHOD(base::RespOrError<json::Json>, getAsJson, (std::string_view ip, const DotPath& path), (override));
};
} // namespace geo::mocks
#endif // _GEO_MOCK_LOCATOR_HPP
