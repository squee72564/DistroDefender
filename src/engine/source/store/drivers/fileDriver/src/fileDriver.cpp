#include <filesystem>

#include "store/drivers/fileDriver.hpp"

#include <base/logger.hpp>

#include <fmt/format.h>

namespace store::drivers
{

FileDriver::FileDriver(const std::filesystem::path& path, bool create)
{
    LOG_DEBUG(
        "Engine file driver init iwth path '{}' and create '{}'.",
        path.string(),
        create
    );

    if (!std::filesystem::exists(path))
    {
        if (!create)
        {
            throw std::runtime_error(
                fmt::format(
                    "Path '{}' does not exist",
                    path.string()
                )
            );
        }

        if (!std::filesystem::create_directories(path))
        {
            throw std::runtime_error(
                fmt::format("Path '{}' cannot be created", path.string())
            );
        }
    }

    if (!std::filesystem::is_directory(path))
    {
        throw std::runtime_error(
            fmt::format(

                "Path '{}' is not a directory",
                path.string()
            )
        );
    }

    path_ = path;
}

std::filesystem::path FileDriver::nameToPath(const base::Name& name) const
{
    std::filesystem::path path{path_};
    for (const auto& part : name.parts())
    {
        path /= part;
    }

    return path;
}

base::OptError FileDriver::createDoc(const base::Name& name, const Doc& content)
{
    auto path = nameToPath(name);

    LOG_DEBUG("FileDriver createDoc name: '{}'.", name.toStr());
    LOG_TRACE("FileDriver createDoc content: '{}'.", content.toStrPretty());

    auto duplicateError = content.checkDuplicateKeys();

    if (duplicateError)
    {
        return base::Error{
            fmt::format(
                "Content '{}' has duplicate keys: {}",
                name.toStr(),
                duplicateError.value().message
            )
        };
    }
    else if (std::filesystem::exists(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' already exists",
                path.string()
            )
        };
    }

    std::error_code ec{};

    if (!std::filesystem::create_directories(path.parent_path(), ec) && ec.value() != 0)
    {
        return base::Error{
            fmt::format(
                "Directory '{}' could not be created: ({}) {}",
                path.parent_path().string(),
                ec.value(),
                ec.message()
            )
        };
    }

    std::ofstream file{path};

    if (!file.is_open())
    {
        return base::Error{
            fmt::format(
                "File '{}' could not be opened on writing mode",
                path.string()
            )
        };
    }

    file << content.toStr();

    return std::nullopt;
}

base::RespOrError<Doc> FileDriver::readDoc(const base::Name& name) const
{
    auto path{nameToPath(name)};

    LOG_DEBUG("FileDriver readDoc name: '{}'.", name.toStr());

    if (!std::filesystem::exists(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' does not exist",
                path.string()
            )
        };
    }

    if (std::filesystem::is_directory(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' is a directory",
                path.string()
            )
        };
    }

    std::ifstream file{path};
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content{buffer.str()};

    try
    {
        return Doc{content.c_str()};
    }
    catch (const std::exception& e)
    {
        return base::Error{
            fmt::format(
                "File '{}' could not be parsed: {}",
                path.string(),
                e.what()
            )
        };
    }
}

base::OptError FileDriver::updateDoc(const base::Name& name, const Doc& content)
{
    auto path = nameToPath(name);

    LOG_DEBUG("FileDriver updateDoc name: '{}'.", name.toStr());
    LOG_TRACE("FileDriver updateDoc content: '{}'.", content.toStrPretty());

    auto duplicateError = content.checkDuplicateKeys();

    if (duplicateError)
    {
        return base::Error{
            fmt::format(
                "Content '{}' has duplicate keys: {}",
                name.toStr(),
                duplicateError.value().message
            )
        };
    }

    if (!std::filesystem::exists(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' does not exist",
                path.string()
            )
        };
    }

    if (std::filesystem::is_directory(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' is a directory",
                path.string()
            )
        };
    }

    std::ofstream file{path};
    
    if (!file.is_open())
    {
        return base::Error{
            fmt::format(
                "File '{}' could not be opened on writing mode",
                path.string()
            )
        };
    }

    file << content.toStr();

    return std::nullopt;
}

base::OptError FileDriver::upsertDoc(const base::Name& name, const Doc& content)
{
    LOG_DEBUG("FileDriver upsertDoc name: '{}'.", name.toStr());

    if (existsDoc(name))
    {
        return updateDoc(name, content);
    }

    return createDoc(name, content);
}

base::OptError FileDriver::removeEmptyParentDirs(const std::filesystem::path& path, const base::Name& name)
{
    std::error_code ec{};
    bool next{true};
    
    for (auto current = path.parent_path();
         next && current != path_ && std::filesystem::is_empty(current);
         current = current.parent_path())
    {
        if (!std::filesystem::remove(current, ec))
        {
            return base::Error{
                fmt::format(
                    "File '{}' was sucessfully removed but its parent directory '{}' could not be removed: ({}) {}",
                    name.toStr(),
                    path.string(),
                    ec.value(),
                    ec.message()
                )
            };
        }
    }

    return std::nullopt;
}

base::OptError FileDriver::deleteDoc(const base::Name& name)
{
    auto path{nameToPath(name)};

    LOG_DEBUG("FileDriver deleteDoc name: '{}'.", name.toStr());

    if (!existsDoc(name))
    {
        return base::Error{
            fmt::format(
                "File '{}' does not exist",
                path.string()
            )
        };
    }

    std::error_code ec{};

    if (!std::filesystem::remove_all(path, ec))
    {
        return base::Error{
            fmt::format(
                "File '{}' could not be removed: ({}) {}",
                path.string(),
                ec.value()
            )
        };
    }

    return removeEmptyParentDirs(path, name);
}

base::RespOrError<Col> FileDriver::readCol(const base::Name& name) const
{
    auto path{nameToPath(name)};

    LOG_DEBUG("FileDriver readCol name: '{}'.", name.toStr());

    if (!std::filesystem::exists(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' does not exist",
                path.string()
            )
        };
    }

    if (!std::filesystem::is_directory(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' is not a directory",
                path.string()
            )
        };
    }

    std::vector<base::Name> names;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        names.emplace_back( base::Name(name) + base::Name(entry.path().filename().string() ) );
    }

    return names;
}

base::RespOrError<Col> FileDriver::readRoot() const
{
    const auto& path = path_;

    LOG_DEBUG("FileDriver readRoot.");

    if (!std::filesystem::exists(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' does not exist", path.string()
            )
        };
    }

    if (!std::filesystem::is_directory(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' is not a directory",
                path.string()
            )
        };
    }

    Col names;

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        names.emplace_back( entry.path().filename().string() );
    }

    return names;
}

base::OptError FileDriver::deleteCol(const base::Name& name)
{
    auto path{nameToPath(name)};

    LOG_DEBUG("FileDriver deleteCol name: '{}'.", name.toStr());

    if (!std::filesystem::exists(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' does not exist",
                path.string()
            )
        };
    }

    if (!std::filesystem::is_directory(path))
    {
        return base::Error{
            fmt::format(
                "File '{}' is not a directory",
                path.string()
            )
        };
    }

    std::error_code ec{};

    if (!std::filesystem::remove_all(path, ec))
    {
        return base::Error{
            fmt::format(
                "File '{}' could not be removed: ({}) {}",
                path.string(),
                ec.value(),
                ec.message()
            )
        };
    }

    return removeEmptyParentDirs(path, name);
}

bool FileDriver::exists(const base::Name& name) const
{
    auto path{nameToPath(name)};

    return std::filesystem::exists(path);
}

bool FileDriver::existsDoc(const base::Name& name) const
{
    auto path{nameToPath(name)};

    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

bool FileDriver::existsCol(const base::Name& name) const
{
    auto path{nameToPath(name)};

    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

} // namespace store::drivers
