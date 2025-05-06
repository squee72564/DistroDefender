#ifndef _ARCHIVER_ARCHIVER_HPP
#define _ARCHIVER_ARCHIVER_HPP

#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <stdexcept>

#include <fmt/format.h>

#include <archiver/iarchiver.hpp>

namespace archiver
{

class Archiver final : public IArchiver
{
private:
    std::filesystem::path filePath_;
    std::ofstream outFile_;
    std::mutex mutex_;
    std::atomic<bool> isActive_;

public:
    explicit Archiver(const std::string& filePath, bool isActive = false);

    ~Archiver() override;

     /**
     * @brief Archive the given data.
     *
     * @param data The data to archive.
     * @return base::OptError An optional error if the archiving fails.
     */
    base::OptError archive(const std::string& data) override;

    /**
     * @brief Activate the archiver.
     *
     */
    void activate() override;

    /**
     * @brief Deactivate the archiver.
     *
     */
    void deactivate() override;

    /**
     * @brief Check if the archiver is active.
     *
     * @return true if the archiver is active, false otherwise.
     */
    bool isActive() const override;

};

} // namespace archiver

#endif //  _ARCHIVER_ARCHIVER_HPP
