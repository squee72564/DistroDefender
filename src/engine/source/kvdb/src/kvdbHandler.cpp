#include <kvdb/kvdbHandler.hpp>

#include <base/json.hpp>
#include <base/logger.hpp>
#include <fmt/format.h>

#include <rocksdb/db.h>


namespace kvdbManager
{

KVDBHandler::KVDBHandler(std::weak_ptr<rocksdb::DB> weakDB,
            std::weak_ptr<rocksdb::ColumnFamilyHandle> weakCFHandle,
            std::shared_ptr<IKVDBHandlerCollection> collection,
            const std::string& dbName,
            const std::string& scopeName)
    : weakDB_{weakDB}
    , weakCFHandle_{weakCFHandle}
    , dbName_{dbName}
    , scopeName_{scopeName}
    , spCollection_{collection}
{}



KVDBHandler::~KVDBHandler()
{
    spCollection_->removeKVDBHandler(dbName_, scopeName_);
}

base::OptError KVDBHandler::set(const std::string& key, const std::string& value)
{
    auto pRocksDB = weakDB_.lock();

    if (!pRocksDB)
    {
        return base::Error{
            "Cannot access RocksDB::DB!"
        };
    }

    auto pCFHandle = weakCFHandle_.lock();

    if (!pCFHandle)
    {
        return base::Error{
            "Cannot access RocksDB Column Family Handle!"
        };
    }

    auto status = pRocksDB->Put(rocksdb::WriteOptions(),
                                pCFHandle.get(), 
                                rocksdb::Slice(key),
                                rocksdb::Slice(value));

    if (!status.ok())
    {
        std::string_view error
            = status.getState() != nullptr ? status.getState() : "Unknown";

        return base::Error{
            fmt::format(
                "Cannot save value '{}' in key '{}'. Error: {}",
                value,
                key,
                error
            )
        };
    }

    return std::nullopt;
}

base::OptError KVDBHandler::set(const std::string& key, const json::Json& value)
{
    return set(key, value.toStr());
}

base::OptError KVDBHandler::add(const std::string& key)
{
    return set(key, "");
}

base::OptError KVDBHandler::remove(const std::string& key)
{
    auto pRocksDB = weakDB_.lock();

    if (!pRocksDB)
    {
        return base::Error{
            "Cannot access RocksDB::DB!"
        };
    }

    auto pCFHandle = weakCFHandle_.lock();

    if (!pCFHandle)
    {
        return base::Error{
            "Cannot access RocksDB Column Family Handle!"
        };
    }

    auto status = pRocksDB->Delete(rocksdb::WriteOptions(),
                                   pCFHandle.get(), 
                                   rocksdb::Slice(key));

    if (!status.ok())
    {
        std::string_view error
            = status.getState() != nullptr ? status.getState() : "Unknown";

        return base::Error{
            fmt::format(
                "Cannot remove key '{}'. Error: {}",
                key,
                error
            )
        };
    }

    return std::nullopt;
}

base::RespOrError<bool> KVDBHandler::contains(const std::string& key)
{
    auto pRocksDB = weakDB_.lock();

    if (!pRocksDB)
    {
        return base::Error{
            "Cannot access RocksDB::DB!"
        };
    }

    auto pCFHandle = weakCFHandle_.lock();

    if (!pCFHandle)
    {
        return base::Error{
            "Cannot access RocksDB Column Family Handle!"
        };
    }

    std::string value{};
    bool valueFound{false};

    try {

        pRocksDB->KeyMayExist(
            rocksdb::ReadOptions(), pCFHandle.get(), rocksdb::Slice(key), &value, &valueFound);

        if (valueFound)
        {
            auto status
                = pRocksDB->Get(
                    rocksdb::ReadOptions(),
                    pCFHandle.get(),
                    rocksdb::Slice(key),
                    &value);

            if (!status.ok())
            {
                valueFound = false;
            }
        }
    }
    catch (const std::exception& ex)
    {
        return base::Error{
            fmt::format(
                "Cannot validate existence of key {}. Error: {}",
                key,
                ex.what()
            )
        };
    }

    return valueFound;
}

base::RespOrError<std::string> KVDBHandler::get(const std::string& key)
{
    auto pRocksDB = weakDB_.lock();

    if (!pRocksDB)
    {
        return base::Error{
            "Cannot access RocksDB::DB!"
        };
    }

    auto pCFHandle = weakCFHandle_.lock();

    if (!pCFHandle)
    {
        return base::Error{
            "Cannot access RocksDB Column Family Handle!"
        };
    }

    std::string value{};

    auto status = pRocksDB->Get(rocksdb::ReadOptions(), pCFHandle.get(), rocksdb::Slice(key), &value);

    if (!status.ok())
    {
        bool isNotFound = status.IsNotFound() && value.empty();
        std::string_view error
            = isNotFound ? "Key not found" : status.getState() != nullptr ? status.getState() : "Unknown";

        return base::Error{
            fmt::format(
                "Cannot get key '{}'. Error: {}",
                key,
                error
            )
        };
    }

    return value;

}

base::RespOrError<std::list<std::pair<std::string, std::string>>>
KVDBHandler::dump(const unsigned int page, const unsigned int records)
{
    return pageContent(page, records);
}

base::RespOrError<std::list<std::pair<std::string, std::string>>>
KVDBHandler::search(const std::string& prefix, const unsigned int page, const unsigned int records)
{
    auto filter = [&prefix](const rocksdb::Slice& keyIter) -> bool
    {
        rocksdb::Slice slicePrefix(prefix);

        if (slicePrefix.empty())
        {
            return true;
        }
        else
        {
            return keyIter.starts_with(slicePrefix);
        }
    };

    return pageContent(page, records, filter);
}

base::RespOrError<std::list<std::pair<std::string, std::string>>>
KVDBHandler::pageContent(const unsigned int page,
            const unsigned int records)
{
    return pageContent(page, records, {});
}

base::RespOrError<std::list<std::pair<std::string, std::string>>>
KVDBHandler::pageContent(const unsigned int page,
            const unsigned int records,
            const std::function<bool(const rocksdb::Slice&)>& filter)
{
    auto pRocksDB = weakDB_.lock();

    if (!pRocksDB)
    {
        return base::Error{
            "Cannot access RocksDB::DB!"
        };
    }

    auto pCFHandle = weakCFHandle_.lock();

    if (!pCFHandle)
    {
        return base::Error{
            "Cannot access RocksDB Column Family Handle!"
        };
    }

    std::unique_ptr<rocksdb::Iterator> iter{
        pRocksDB->NewIterator(rocksdb::ReadOptions(), pCFHandle.get())
    };

    std::list<std::pair<std::string, std::string>> content;

    unsigned int fromRecords = (page-1) * records;
    unsigned int toRecords = fromRecords + records;

    unsigned int i = 0;

    for (iter->SeekToFirst(); iter->Valid() && i < toRecords; iter->Next())
    {
        if (!filter || filter(iter->key()))
        {
            if (i >= fromRecords)
            {
                content.emplace_back(
                    iter->key().ToString(),
                    iter->value().ToString()
                );
            }

            ++i;
        }

    }

    if (!iter->status().ok())
    {
        return base::Error{
            fmt::format(
                "Database '{}': Could not iterate over database: '{}'",
                dbName_,
                iter->status().ToString()
            )
        };
    }

    return content;
}

} // namespace kvdbManager
