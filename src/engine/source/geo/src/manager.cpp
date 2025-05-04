#include <geo/manager.hpp>

#include <filesystem>
#include <fstream>
#include <mutex>

#include <fmt/format.h>
#include <maxminddb.h>

#include <base/logger.hpp>
#include <store/istore.hpp>

#include <geo/dbEntry.hpp>
#include <geo/locator.hpp>

namespace geo
{

Manager::Manager(const std::shared_ptr<store::IStoreInternal>& store, const std::shared_ptr<IDownloader>& downloader)
    : dbs_{}
    , dbTypes_{}
    , rwMapMutex_{}
    , store_{store}
    , downloader_{downloader}
{
    if (store_ == nullptr)
    {
        throw std::runtime_error("Maxminddb manager needs a non-null store");
    }

    if (downloader_ == nullptr)
    {
        throw std::runtime_error("Maxminddb manager needs a non-null downloader");
    }

    auto dbsResp = store_->readInternalCol(INTERNAL_NAME);

    if (base::isError(dbsResp))
    {
        LOG_DEBUG(
            "Geo module does not have DBs in the store: {}",
            base::getError(dbsResp).message
        );
        return;
    }

    auto dbs = base::getResponse(dbsResp);

    for (const auto& db : dbs)
    {
        auto dbResp = store_->readInternalDoc(db);
        if (base::isError(dbResp))
        {
            LOG_DEBUG(
                "Geo cannot read internal document '{}': '{}'",
                db,
                base::getError(dbResp).message
            );
            continue;
        }

        auto doc = base::getResponse(dbResp);

        std::string path{};
        std::string typeStr{};

        try
        {
            path = doc.getString(PATH_PATH).value();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(
                fmt::format(
                    "Could not get path from document at '{}': {}\nDocument: {}",
                    PATH_PATH,
                    e.what(),
                    doc.toStrPretty()
                )
            );
        }

        try
        {
            typeStr = doc.getString(TYPE_PATH).value();
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(
                fmt::format(
                    "Could not get type from document at '{}': {}\nDocument: {}",
                    TYPE_PATH,
                    e.what(),
                    doc.toStrPretty()
                )
            );
        }

        auto type = typeFromName(typeStr);

        auto addResp = addDbUnsafe(path, type, false);
        if (base::isError(addResp))
        {
            LOG_ERROR(
                "Geo cannot add db '{}': {}",
                path,
                base::getError(addResp).message
            );

            store_->deleteInternalDoc(db);
            LOG_TRACE("Geo deleted internal document '{}'", db);
        }
    }

}

base::OptError Manager::upsertStoreEntry(std::string_view path)
{
    std::filesystem::path dbPath{path};

    auto file = std::ifstream(path, std::ios::binary);
    if (!file.is_open())
    {
        return base::Error{
            fmt::format("Cannot open file '{}'", path)
        };
    }

    std::string content{
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    };

    auto hashResp = downloader_->computeMD5(content);

    if (base::isError(hashResp))
    {
        return base::Error{
            fmt::format(
                "Error computing MD5 hash: {}",
                base::getError(hashResp).message
            )
        };
    }

    auto hash = base::getResponse(hashResp);

    auto internalName =
        base::Name( std::vector<std::string>( {std::string(INTERNAL_NAME), dbPath.filename().string()} ) );

    auto doc = store::Doc();
    
    doc.setType(PATH_PATH, path);
    doc.setType(HASH_PATH, hash);
    doc.setType(TYPE_PATH, typeName(dbs_.at(dbPath.filename().string())->type));

    return store_->upsertInternalDoc(internalName, doc);
}

base::OptError Manager::removeInternalEntry(std::string_view path)
{
    auto internalName = base::Name{
        fmt::format(
            "{}{}{}",
            INTERNAL_NAME,
            base::Name::SEPARATOR_S,
            std::filesystem::path(path).filename().string()
        )
    };

    return store_->deleteInternalDoc(internalName);
}

base::OptError Manager::addDbUnsafe(std::string_view path, Type type, bool upsertStore)
{
    auto name = std::filesystem::path(path).filename().string();

    if (dbTypes_.find(type) != dbTypes_.end())
    {
        return base::Error{
            fmt::format(
                "Type '{}' already has the database '{}'",
                typeName(type),
                dbTypes_.at(type)
            )
        };
    }

    if (dbs_.find(name) != dbs_.end())
    {
        return base::Error{
            fmt::format(
                "Database with name '{}' already exists",
                name
            )
        };
    }

    auto entry = std::make_shared<DbEntry>(path, type);
    int status = MMDB_open(path.data(), MMDB_MODE_MMAP, entry->mmdb.get());

    if (MMDB_SUCCESS != status)
    {
        return base::Error{
            fmt::format(
                "Cannot add database '{}': {}",
                path,
                MMDB_strerror(status)
            )
        };
    }

    dbs_.emplace(name, std::move(entry));
    dbTypes_.emplace(type, name);

    if (upsertStore)
    {
        auto internalResp = upsertStoreEntry(path);
        if (base::isError(internalResp))
        {
            LOG_WARNING(
                "Cannot update internal store for '{}': {}",
                path,
                base::getError(internalResp).message
            );
        }
    }

    return base::noError();
}

base::OptError Manager::removeDbUnsafe(std::string_view path)
{
    auto name = std::filesystem::path(path).filename().string();
    
    std::shared_ptr<DbEntry> entry;

    {
        auto it = dbs_.find(name);
        if (it == dbs_.end())
        {
            return base::Error{
                fmt::format(
                    "Database '{}' not found",
                    name
                )
            };
        }
        
        entry = it->second;
    }

    if (!entry)
    {
        return base::Error{
            fmt::format(
                "Entry for database '{}' is null", name
            )
        };
    }

    try
    {
        std::unique_lock<std::shared_mutex> lockEntry(entry->rwMutex);

        dbs_.erase(name);

        //lockEntry.unlock();

        //entry.reset();

        for (auto it = dbTypes_.begin(); it != dbTypes_.end(); ++it)
        {
            if (it->second == name)
            {
                dbTypes_.erase(it);
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            fmt::format(
                "unique_lock failed for rwMapMutex_ in {}: {}",
                __func__,
                e.what()
            )
        );
    }


    return removeInternalEntry(path);
}

base::OptError Manager::writeDb(std::string_view path, std::string_view content)
{
    auto filePath = std::filesystem::path(path);

    try
    {
        std::filesystem::create_directories(filePath.parent_path());
    }
    catch (const std::exception& e)
    {
        return base::Error{
            fmt::format(
                "Cannot create directories for '{}': {}",
                path,
                e.what()
            )
        };
    }

    std::ofstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        return base::Error{
            fmt::format(
                "Cannot open file '{}'",
                path
            )
        };
    }

    try
    {
        file.write(content.data(), content.size());
    }
    catch (const std::exception& e)
    {
        return base::Error{
            fmt::format(
                "Cannot write to file '{}': {}",
                path,
                e.what()
            )
        };
    }

    return base::noError();
}

base::OptError Manager::addDb(std::string_view path, Type type)
{
    
    try
    {
        std::unique_lock<std::shared_mutex> lock(rwMapMutex_);

        auto resp = addDbUnsafe(path, type, true);

        return resp;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            fmt::format(
                "unique_lock failed for rwMapMutex_ in {}: {}",
                __func__,
                e.what()
            )
        );
    }
}

base::OptError Manager::removeDb(std::string_view path)
{
    try
    {
        std::unique_lock<std::shared_mutex> lock(rwMapMutex_);
        
        auto resp = removeDbUnsafe(path);

        return resp;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            fmt::format(
                "unique_lock failed for rwMapMutex_ in {}: {}",
                __func__,
                e.what()
            )
        );
    }
}

base::OptError
Manager::remoteUpsertDb(std::string_view path, Type type, std::string_view dbUrl, std::string_view hashUrl)
{
    auto name = std::filesystem::path(path).filename().string();

    std::unique_lock<std::shared_mutex> lock(rwMapMutex_);

    if (dbTypes_.find(type) != dbTypes_.end() && dbTypes_.at(type) != name)
    {
        return base::Error{
            fmt::format(
                "The name '{}' does not correspond to any databse for type '{}' "
                "If you want it to correspond, please delete the existing databse "
                "and recreate it with this name.",
                name,
                typeName(type)
            )
        };
    }

    auto hashResp = downloader_->downloadMD5(hashUrl);
    if (base::isError(hashResp))
    {
        return base::Error{
            fmt::format(
                "Cannot download hash from '{}': {}",
                hashUrl,
                base::getError(hashResp).message
            )
        };
    }

    auto hash = base::getResponse(hashResp);

    auto entry = dbs_.find(name);
    if (entry != dbs_.end())
    {
        auto internalResp = store_->readInternalDoc(
                                base::Name{
                                    fmt::format(
                                        "{}{}{}",
                                        INTERNAL_NAME,
                                        base::Name::SEPARATOR_S,
                                        name
                                    )
                                }
                            );
        if (!base::isError(internalResp))
        {
            auto storedHash = base::getResponse(internalResp).getString(HASH_PATH).value();
            if (storedHash == hash)
            {
                return base::noError();
            }
        }
    }

    std::string content{};
    base::OptError error;

    for (int i = 0; i < MAX_RETRIES; ++i)
    {
        auto dbResp = downloader_->downloadHTTPS(dbUrl);

        if (base::isError(dbResp))
        {
            error = base::Error{
                fmt::format(
                    "Cannot download database from '{}': {}",
                    dbUrl,
                    base::getError(dbResp).message
                )
            };
            continue;
        }

        content = base::getResponse(dbResp);
        auto computedHashResp = downloader_->computeMD5(content);

        if (base::isError(computedHashResp))
        {
            error = base::Error{
                fmt::format(
                    "Cannot compute hash from database downloaded at '{}' : {}",
                    dbUrl,
                    base::getError(computedHashResp).message
                )
            };
        }

        auto computedHash = base::getResponse(computedHashResp);

        if (computedHash == hash)
        {
            error = base::noError();
            break;
        }
    }

    if (base::isError(error))
    {
        return base::getError(error);
    }

    if (entry != dbs_.end())
    {
        try
        {
            std::unique_lock<std::shared_mutex> lockEntry(entry->second->rwMutex);
            auto writeResp = writeDb(path, content);
            if (base::isError(writeResp))
            {
                return base::getError(writeResp);
            }

            MMDB_close(entry->second->mmdb.get());

            int status = MMDB_open(path.data(), MMDB_MODE_MMAP, entry->second->mmdb.get());

            if (MMDB_SUCCESS != status)
            {
                lockEntry.unlock();
                removeDbUnsafe(path);

                return base::Error{
                    fmt::format(
                        "Cannot add database '{}': {}",
                        path,
                        MMDB_strerror(status)
                    )
                };
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(
                fmt::format(
                    "unique_lock failed for rwMapMutex_ in {}: {}",
                    __func__,
                    e.what()
                )
            );
        }

    }
    else
    {
        auto writeResp = writeDb(path, content);
        if (base::isError(writeResp))
        {
            return base::getError(writeResp);
        }

        auto addResp = addDbUnsafe(path, type, false);
        if (base::isError(addResp))
        {
            return base::getError(addResp);
        }
    }

    auto internalResp = upsertStoreEntry(path);
    if (base::isError(internalResp))
    {
        LOG_WARNING(
            "Cannot update internal store for '{}': {}",
            path,
            base::getError(internalResp).message
        );
    }
    
    return base::noError();
}

base::RespOrError<std::shared_ptr<ILocator>> Manager::getLocator(Type type) const
{
    std::shared_lock lock{rwMapMutex_};

    if (dbTypes_.find(type) == dbTypes_.end())
    {
        return base::Error{
            fmt::format(
                "Type '{}' does not have a database",
                typeName(type)
            )
        };
    }

    auto entry = dbs_.at(dbTypes_.at(type));

    auto locator = std::make_shared<Locator>(entry);

    return locator;
}

std::vector<DbInfo> Manager::listDbs() const
{
    std::shared_lock lock{rwMapMutex_};

    std::vector<DbInfo> dbs;

    for (const auto& [name, entry] : dbs_)
    {
        dbs.emplace_back( DbInfo{name, entry->path, entry->type} ); 
    }

    return dbs;
}

} // namespace geo
