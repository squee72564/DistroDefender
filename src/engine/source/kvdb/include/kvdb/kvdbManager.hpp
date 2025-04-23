#ifndef _KVDB_MANAGER_HPP
#define _KVDB_MANAGER_HPP

#include <atomic>
#include <filesystem>
#include <map>
#include <mutex>

#include <rocksdb/db.h>
#include <rocksdb/options.h>

#include <base/error.hpp>
#include <kvdb/ikvdbmanager.hpp>
#include <kvdb/kvdbHandler.hpp>
#include <kvdb/kvdbHandlerCollection.hpp>

namespace kvdbManager
{

constexpr static const char* DEFAULT_CF_NAME {"default"};

struct KVDBManagerOptions
{
    std::filesystem::path dbStoragePath;
    std::string dbName;
};

class KVDBManager final : public IKVDBManager
{
public:
    KVDBManager(const KVDBManager&) = delete;
    KVDBManager(KVDBManager&&) = delete;

    const KVDBManager& operator=(const KVDBManager&) = delete;
    const KVDBManager& operator=(KVDBManager&&) = delete;

    KVDBManager(const KVDBManagerOptions& options);
    ~KVDBManager() override = default;

    void initialize() override;

    void finalize() override;

    std::map<std::string, RefInfo> getKVDBScopesInfo() override;

    std::map<std::string, RefInfo> getKVDBHandlersInfo() const override;

    uint32_t getKVDBHandlersCount(const std::string& dbName) const override;

    base::RespOrError<std::shared_ptr<IKVDBHandler>>
    getKVDBHandler(const std::string& dbName, const std::string& scopeName) override;

    std::vector<std::string> listDBs(const bool loaded) override;

    base::OptError deleteDB(const std::string& name) override;

    base::OptError createDB(const std::string& name) override;

    base::OptError createDB(const std::string& name, const std::string& path) override;

    base::OptError loadDBFromJson(const std::string& name, const json::Json& content) override;

    bool existsDB(const std::string& name) override;

private:

    void initializeOptions();

    void initializeMainDB();

    void finalizeMainDB();

    base::RespOrError<json::Json> getContentFromJsonFile(const std::string& path);

    std::shared_ptr<rocksdb::ColumnFamilyHandle>
    createSharedCFHandle(rocksdb::ColumnFamilyHandle* cfRawPtr);

    base::OptError createColumnFamily(const std::string& name);

    std::shared_ptr<KVDBHandlerCollection> kvdbHandlerCollection_;
    
    KVDBManagerOptions managerOptions_;

    rocksdb::Options rocksDBOptions_;

    std::shared_ptr<rocksdb::DB> pRocksDB_;

    std::map<std::string, std::shared_ptr<rocksdb::ColumnFamilyHandle>> mapCFHandles_;

    std::shared_ptr<rocksdb::ColumnFamilyHandle> pDefaultCFHandle_;

    std::mutex mutexScopes_;

    std::atomic<bool> isInitialized_{false};
};

} // namespace kvdbManager

#endif // _KVDB_MANAGER_HPP
