#ifndef _GEO_MANAGER_HPP
#define _GEO_MANAGER_HPP

#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>

#include <geo/idownloader.hpp>
#include <geo/imanager.hpp>
#include <store/istore.hpp>

namespace geo
{

/**
 * @brief Class to hold the needed information for a database.
 */
class DbEntry;

auto constexpr MAX_RETRIES = 3;
constexpr std::string_view INTERNAL_NAME = "geo";
constexpr std::string_view PATH_PATH = "/path";
constexpr std::string_view HASH_PATH = "/hash";
constexpr std::string_view TYPE_PATH = "/type";

class Manager final : public IManager
{
private:
    std::map<std::string, std::shared_ptr<DbEntry>> dbs_; ///< The databases that have been added.
    std::map<Type, std::string> dbTypes_;  ///< Map by Types for quick access to the db name. (only one db per type)
    mutable std::shared_mutex rwMapMutex_; ///< Mutex to avoid simultaneous updates on the db map

    std::shared_ptr<store::IStoreInternal> store_; ///< The store used to store the MMDB hash.
    std::shared_ptr<IDownloader> downloader_;      ///< The downloader used to download the MMDB database.

    /**
     * @brief Upsert the internal store entry for a database.
     *
     * @param path The path to the database.
     * @return base::OptError An error if the store entry could not be upserted.
     */
    base::OptError upsertStoreEntry(std::string_view path);

    /**
     * @brief Remove the internal store entry for a database.
     *
     * @param path The path to the database.
     * @return base::OptError An error if the store entry could not be removed.
     */
    base::OptError removeInternalEntry(std::string_view path);

    /**
     * @brief Add a database to the manager without any thread safety checks.
     *
     * @param path Path to the database.
     * @param type Type of the database.
     * @param upsertStore Whether to upsert the store entry.
     * @return base::OptError An error if the database could not be added.
     */
    base::OptError addDbUnsafe(std::string_view path, Type type, bool upsertStore);

    /**
     * @brief Remove a database from the manager without any thread safety checks.
     *
     * @param path Path to the database.
     * @return base::OptError An error if the database could not be removed.
     */
    base::OptError removeDbUnsafe(std::string_view path);

    /**
     * @brief Write the MMDB database to the filesystem.
     *
     * @param path Path to store the database.
     * @param content The content of the database.
     * @return base::OptError An error if the database could not be written.
     */
    base::OptError writeDb(std::string_view path, std::string_view content);

public:
    ~Manager() override = default;

    Manager() = delete;
    Manager(const std::shared_ptr<store::IStoreInternal>& store, const std::shared_ptr<IDownloader>& downloader);

    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;

    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;

    /**
     * @copydoc IManager::listDbs
     */
    std::vector<DbInfo> listDbs() const override;

    /**
     * @copydoc IManager::addDb
     */
    base::OptError addDb(std::string_view path, Type type) override;

    /**
     * @copydoc IManager::removeDb
     */
    base::OptError removeDb(std::string_view path) override;

    /**
     * @copydoc IManager::remoteUpsertDb
     */
    base::OptError
    remoteUpsertDb(std::string_view path, Type type, std::string_view dbUrl, std::string_view hashUrl) override;

    /**
     * @copydoc IManager::getLocator
     */
    base::RespOrError<std::shared_ptr<ILocator>> getLocator(Type type) const override;
};

} // namespace geo
#endif // _GEO_MANAGER_HPP
