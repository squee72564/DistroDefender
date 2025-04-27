#ifndef _FILE_DRIVER_HPP
#define _FILE_DRIVER_HPP

#include <store/idriver.hpp>

#include <filesystem>
#include <fstream>

namespace store::drivers
{

class FileDriver : public IDriver
{
private:
    std::filesystem::path path_;

    std::filesystem::path nameToPath(const base::Name& name) const;

    base::OptError removeEmptyParentDirs(const std::filesystem::path& path, const base::Name& name);

public:
    FileDriver(const std::filesystem::path& path, bool create = false);
    ~FileDriver() = default;

    FileDriver(const FileDriver&) = delete;
    FileDriver& operator=(const FileDriver&) = delete;

    base::OptError createDoc(const base::Name& name, const json::Json& content) override;

    base::RespOrError<Doc> readDoc(const base::Name& name) const override;

    base::OptError updateDoc(const base::Name& name, const json::Json& content) override;

    base::OptError upsertDoc(const base::Name& name, const json::Json& content) override;

    base::OptError deleteDoc(const base::Name& name) override;

    base::RespOrError<Col> readCol(const base::Name& name) const override;

    base::RespOrError<Col> readRoot() const override;

    base::OptError deleteCol(const base::Name& name) override;

    bool exists(const base::Name& name) const override;

    bool existsDoc(const base::Name& name) const override;

    bool existsCol(const base::Name& name) const override;
};

} // namespace store::drivers

#endif // _FILE_DRIVER_HPP
