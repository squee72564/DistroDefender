#include <archiver/archiver.hpp>

namespace archiver
{

    Archiver::Archiver(const std::string& filePath, bool isActive)
        : filePath_{filePath}
        , outFile_{}
        , mutex_{}
        , isActive_{isActive}
    {
        if (!std::filesystem::exists(filePath_.parent_path()) ||
            !std::filesystem::is_directory(filePath_.parent_path()))
        {
            throw std::runtime_error(
                fmt::format(
                    "Archiver directory does not exist: {}",
                    filePath_.parent_path().string()
                )
            );
        }

        outFile_.open(filePath_, std::ios::out | std::ios::app);

        if (!outFile_.is_open())
        {
            throw std::runtime_error(
                fmt::format(
                    "Archiver failed to open file: {}",
                    filePath_.string()
                )
            );
        }
    }

    Archiver::~Archiver()
    {
        if (outFile_.is_open())
        {
            outFile_.close();
        }
    }

    base::OptError Archiver::archive(const std::string& data)
    {
        if (!isActive_.load())
        {
            return base::noError();
        }

        std::lock_guard<std::mutex> lock(mutex_);

        if (!outFile_.is_open())
        {
            return base::Error{
                fmt::format(
                    "File is not open: {}",
                    filePath_.string()
                )
            };
        }

        outFile_ << data << "\n";

        if (outFile_.fail())
        {
            return base::Error{
                fmt::format(
                    "Failed to write to file: {}",
                    filePath_.string()
                )
            };
        }

        outFile_.flush();

        return base::noError();
    }

    void Archiver::activate()
    {
        isActive_.store(true);
    }

    void Archiver::deactivate()
    {
        isActive_.store(false);
    }

    bool Archiver::isActive() const
    {
        return isActive_.load();
    }

} // namespace archiver
