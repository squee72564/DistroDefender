#ifndef _I_KVDB_MANAGER_HPP
#define _I_KVDB_MANAGER_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <kvdb/ikvdbhandler.hpp>

namespace kvdbManager
{

using RefInfo = std::map<std::string, uint32_t>;

class IKVDBManager
{
public:
    virtual void initialize() = 0;

    virtual void finalize() = 0;

    virtual std::vector<std::string> listDBs(const bool loaded) = 0;

    virtual base::OptError deleteDB(const std::string& name) = 0;

    virtual base::OptError createDB(const std::string& name) = 0;

    virtual base::OptError createDB(const std::string& name, const std::string& path) = 0;

    virtual base::OptError loadDBFromJson(const std::string& name, const json::Json content) = 0;

    virtual bool existsDB(const std::string& name) = 0;

    virtual std::map<std::string, RefInfo> getKVDBScopesInfo() = 0;

    virtual std::map<std::string, RefInfo> getKVDBHandlersInfo() const = 0;

    virtual base::RespOrError<std::shared_ptr<IKVDBHandler>>
    getKVDBHandler(const std::string& dbName, const std::string& scopeName) = 0;

    virtual uint32_t getKVDBHandlersCount(const std::string& dbName) const = 0;
};

} // namepsace kvdbManager

#endif //_I_KVDB_MANAGER_HPP
