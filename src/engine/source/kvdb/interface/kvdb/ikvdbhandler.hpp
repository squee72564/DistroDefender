#ifndef _I_KVDB_HANDLER_HPP
#define _I_KVDB_HANDLER_HPP

#include <list>
#include <unordered_map>
#include <string>
#include <utility>
#include <variant>

#include <base/error.hpp>
#include <base/json.hpp>

namespace kvdbManager
{

class IKVDBHandler
{
public:
    virtual ~IKVDBHandler() = default;

    virtual base::OptError
    set(const std::string& key, const std::string& value) = 0;

    virtual base::OptError
    set(const std::string& key, const json::Json& value) = 0;

    virtual base::OptError
    add(const std::string& key) = 0;

    virtual base::OptError
    remove(const std::string& key) = 0;

    virtual base::RespOrError<bool>
    contains(const std::string& key) = 0;

    virtual base::RespOrError<std::string>
    get(const std::string& key) = 0;


    virtual base::RespOrError<std::list<std::pair<std::string, std::string>>>
    dump(const unsigned int page, const unsigned int records) = 0;

    inline base::RespOrError<std::list<std::pair<std::string, std::string>>> dump()
    {
        return dump(0, 0);
    };

    virtual base::RespOrError<std::list<std::pair<std::string, std::string>>>
    search(const std::string& prefiex, const unsigned int page, const unsigned int records) = 0;

    inline base::RespOrError<std::list<std::pair<std::string, std::string>>>
    search(const std::string& prefix)
    {
        return search(prefix, 0, 0);
    };

};

} // namespace kvdbManager

#endif // _I_KVDB_HANDLER_HPP
