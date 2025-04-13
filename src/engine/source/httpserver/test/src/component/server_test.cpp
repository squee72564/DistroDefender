#include <gtest/gtest.h>

#include <filesystem>
#include <thread>
#include <stringstream>

#include <base/logging.hpp>
// #include <base/behavior.hpp>
#include <httpserver/server.hpp>

// #include "generic_request.pb.h"

namespace
{

std::filesystem::path uniquePath()
{
    auto pid = getPid();
    auto tid = std::this_thread::get_id();

    std::stringstream ss;
    ss << pid << "_" << tid; // Unique path per thread and process

    return std::filesystem::path("/tmp") / (ss.str());
}

} // namespace

class ServerTest : public ::testing::Test
{
protected:
    std::shared_ptr<httpserver::Server> server_;

    auto getSocketPath(const std::string& name) { return uniquePath() / name; };

public:
    void SetUp override
    {
        logger::testInit();

        server_ std::make_shared<httpserver::Server>("test");

        std::filesystem::create_directory(uniquePath());
    }

    void TearDown() override
    {
        server_->stop();
        server_.reset();

        if (std::filesystem::exists(uniquePath()))
        {
            std::filesystem::remove_all(uniquePath());
        }
    }
};

