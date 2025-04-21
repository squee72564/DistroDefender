#include <fmt/format.h>

#include <base/logger.hpp>

#include <kvdb/kvdbHandlerCollection.hpp>

namespace kvdbManager
{

void KVDBHandlerInstance::addScope(const std::string& scopeName)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    scopeCounter_.addRef(scopeName);
}

void KVDBHandlerInstance::removeScope(const std::string& scopeName)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    scopeCounter_.removeRef(scopeName);
}

bool KVDBHandlerInstance::emptyScopes()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    return scopeCounter_.empty();
}

std::vector<std::string> KVDBHandlerInstance::getRefNames()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    return scopeCounter_.getRefNames();
}

std::map<std::string, uint32_t> KVDBHandlerInstance::getRefMap()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    return scopeCounter_.getRefMap();
}


void KVDBHandlerCollection::addKVDBHandler(const std::string &dbName, const std::string& scopeName)
{
    {
        std::shared_lock<std::shared_mutex> readLock(mutex_);
        const auto it = mapInstances_.find(dbName);
        if (it != mapInstances_.end())
        {
            it->second->addScope(scopeName);
            return;
        }
    }

    {
        std::unique_lock<std::shared_mutex> writeLock(mutex_);
        auto [it, inserted]
            = mapInstances_.try_emplace(dbName, std::make_shared<KVDBHandlerInstance>());
        it->second->addScope(scopeName);
    }
}

void KVDBHandlerCollection::removeKVDBHandler(const std::string& dbName, const std::string& scopeName)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    const auto it = mapInstances_.find(dbName);

    if (it != mapInstances_.end())
    {
        auto& instance = it->second;
        instance->removeScope(scopeName);
        if (instance->emptyScopes())
        {
            mapInstances_.erase(it);
        }
    }
}

std::vector<std::string> KVDBHandlerCollection::getDBNames()
{
    std::shared_lock<std::shared_mutex>lock(mutex_);

    std::vector<std::string> dbNames;
    dbNames.reserve(mapInstances_.size());

    for (const auto& instance : mapInstances_)
    {
        dbNames.push_back(instance.first);
    }

    return dbNames;
}

std::map<std::string, uint32_t> KVDBHandlerCollection::getRefMap(const std::string& dbName)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto it = mapInstances_.find(dbName);

    if (it != mapInstances_.end())
    {
        return it->second->getRefMap();
    }

    return {};
}

} // namespace kvdbManager
