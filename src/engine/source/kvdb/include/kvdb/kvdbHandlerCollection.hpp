#ifndef _KVDB_HANDLER_COLLECTION_HPP
#define _KVDB_HANDLER_COLLECTION_HPP

#include <map>
#include <memory>
#include <set>
#include <shared_mutex>

#include <kvdb/ikvdbhandlercollection.hpp>
#include <kvdb/kvdbHandler.hpp>
#include <kvdb/refCounter.hpp>

namespace kvdbManager
{

/**
 * @brief Helper class to manage the reference counters for a given DB.
 * This is used to track how many scopes are using a given DB.
 */
class KVDBHandlerInstance
{
public:
    void addScope(const std::string& scopeName);

    void removeScope(const std::string& scopeName);

    bool emptyScopes();

    std::vector<std::string> getRefNames();

    std::map<std::string, uint32_t> getRefMap();

private:
    RefCounter scopeCounter_;

    std::shared_mutex mutex_;
};

class KVDBHandlerCollection : public IKVDBHandlerCollection
{
public:
    virtual ~KVDBHandlerCollection() = default;

    void addKVDBHandler(const std::string &dbName, const std::string& scopeName) override;

    void removeKVDBHandler(const std::string& dbName, const std::string& scopeName) override;

    std::vector<std::string> getDBNames();

    std::map<std::string, uint32_t> getRefMap(const std::string& dbName);

private:
    
    std::map<std::string, std::shared_ptr<KVDBHandlerInstance>> mapInstances_;

    std::shared_mutex mutex_;
};

} // namespace kvdbManager

#endif // _KVDB_HANDLER_COLLECTION_HPP
