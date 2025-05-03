
#ifndef _GEO_MOCK_MANAGER_HPP
#define _GEO_MOCK_MANAGER_HPP

#include <gmock/gmock.h>

#include <geo/imanager.hpp>

namespace geo::mocks
{
class MockManager : public IManager
{
public:
    MOCK_METHOD(base::OptError, addDb, (std::string_view path, Type type), (override));
    MOCK_METHOD(base::OptError, removeDb, (std::string_view path), (override));
    MOCK_METHOD(base::OptError,
                remoteUpsertDb,
                (std::string_view path, Type type, std::string_view dbUrl, std::string_view hashUrl),
                (override));
    MOCK_METHOD(std::vector<DbInfo>, listDbs, (), (const, override));
    MOCK_METHOD(base::RespOrError<std::shared_ptr<ILocator>>, getLocator, (Type type), (const, override));
};
} // namespace geo::mocks
#endif // _GEO_MOCK_MANAGER_HPP

