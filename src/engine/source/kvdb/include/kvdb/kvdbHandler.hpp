#ifndef _KVDB_HANDLER_HPP
#define _KVDB_HANDLER_HPP

#include <kvdb/ikvdbhandler.hpp>
#include <kvdb/ikvdbhandlercollection.hpp>

#include <rocksdb/slice.h>

// Forward declaration for RocksDB types used
namespace rocksdb
{
class DB;
class ColumnFamilyHandle;
} // namespace rocksdb

namespace kvdbManager
{

class IKVDBHandlerCollection;

class KVDBHandler : public IKVDBHandler
{
public:
    KVDBHandler(std::weak_ptr<rocksdb::DB> weakDB,
                std::weak_ptr<rocksdb::ColumnFamilyHandle> weakCFHandle,
                std::shared_ptr<IKVDBHandlerCollection> collection,
                const std::string& dbName,
                const std::string& scopeName);

    ~KVDBHandler() override;

    base::OptError set(const std::string& key, const std::string& value) override;

    base::OptError set(const std::string& key, const json::Json& value) override;

    base::OptError add(const std::string& key) override;

    base::OptError remove(const std::string& key) override;

    base::RespOrError<bool> contains(const std::string& key) override;

    base::RespOrError<std::string> get(const std::string& key) override;

    base::RespOrError<std::list<std::pair<std::string, std::string>>>
    dump(const unsigned int page, const unsigned int records) override;

    base::RespOrError<std::list<std::pair<std::string, std::string>>>
    search(const std::string& prefix, const unsigned int page, const unsigned int records) override;

protected:
    std::weak_ptr<rocksdb::DB> weakDB_;
    std::weak_ptr<rocksdb::ColumnFamilyHandle> weakCFHandle_;
    std::string dbName_;
    std::string scopeName_;
    std::shared_ptr<IKVDBHandlerCollection> spCollection_;

private:
    base::RespOrError<std::list<std::pair<std::string, std::string>>>
    pageContent(const unsigned int page,
                const unsigned int records);

    base::RespOrError<std::list<std::pair<std::string, std::string>>>
    pageContent(const unsigned int page,
                const unsigned int records,
                const std::function<bool(const rocksdb::Slice&)>& filter);
};

} // namespace kvdbManager

#endif // _KVDB_HANDLER_HPP
