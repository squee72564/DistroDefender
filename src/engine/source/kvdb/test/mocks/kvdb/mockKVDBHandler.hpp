#ifndef _KVDB_MOCK_HANDLER_HPP
#define _KVDB_MOCK_HANDLER_HPP

#include <gmock/gmock.h>

#include <kvdb/ikvdbhandler.hpp>

namespace kvdb::mocks
{

inline base::OptError kvdbSetOk()
{
    return std::nullopt;
}

inline base::RespOrError<bool> kvdbContainsOk()
{
    return true;
}

inline base::RespOrError<bool> kvdbContainsNOk()
{
    return false;
}

inline base::RespOrError<bool> kvdbContainsError()
{
    return base::Error{"Mocked KVDB error"};
}

inline base::RespOrError<std::string> kvdbGetOk()
{
    return R"("value")";
}

inline base::RespOrError<std::string> kvdbGetOk(const std::string& res)
{
    return res;
}

inline base::RespOrError<std::string> kvdbGetError(const std::string& err)
{
    return base::Error{ err };
}

inline base::RespOrError<std::list<std::pair<std::string, std::string>>> kvdbDumpOk()
{
    return {};
}

/**************
 * Mock Classes
 **************/

class MockKVDBHandler : public kvdbManager::IKVDBHandler
{
public:
    virtual ~MockKVDBHandler() = default;

    MOCK_METHOD((base::OptError), set, (const std::string& key, const std::string& value), (override));
    MOCK_METHOD((base::OptError), set, (const std::string& key, const json::Json& value), (override));
    MOCK_METHOD((base::OptError), add, (const std::string& key), (override));
    MOCK_METHOD((base::OptError), remove, (const std::string& key), (override));
    MOCK_METHOD((base::RespOrError<bool>), contains, (const std::string& key), (override));
    MOCK_METHOD((base::RespOrError<std::string>), get, (const std::string& key), (override));
    MOCK_METHOD((base::RespOrError<std::list<std::pair<std::string, std::string>>>),
                dump,
                (const unsigned int page, const unsigned int records),
                (override));
    MOCK_METHOD((base::RespOrError<std::list<std::pair<std::string, std::string>>>), dump, (), ());
    MOCK_METHOD((base::RespOrError<std::list<std::pair<std::string, std::string>>>),
                search,
                (const std::string& prefix, const unsigned int page, const unsigned int records),
                (override));
    MOCK_METHOD((base::RespOrError<std::list<std::pair<std::string, std::string>>>),
                search,
                (const std::string& prefix),
                ());
};


} // namespace kvdb::mocks

#endif  // _KVDB_MOCK_HANDLER_HPP
