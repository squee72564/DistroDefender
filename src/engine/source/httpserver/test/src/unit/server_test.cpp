#include <gtest/gtest.h>

#include <csignal>

#include <base/logger.hpp>
#include <httpserver/server.hpp>

namespace
{

std::filesystem::path uniquePath()
{
    auto pid = getpid();
    auto tid = std::this_thread::get_id();

    std::stringstream ss;
    ss << pid << "_" << tid; // Unique path per thread and process

    return std::filesystem::path("/tmp") / (ss.str());
}

} // namespace

class ServerTest : public ::testing::Test
{
protected:
    auto getSocketPath(const std::string& name) { return uniquePath() / name; }

public:
    void SetUp() override
    {
        logger::testInit();
        std::filesystem::create_directory(uniquePath());
    }

    void TearDown() override
    {
        if (std::filesystem::exists(uniquePath()))
        {
            std::filesystem::remove_all(uniquePath());
        }
    }
};

TEST_F(ServerTest, Create)
{
    EXPECT_NO_THROW(httpserver::Server server("test"));
}

TEST_F(ServerTest, StartEmptySocketPath)
{
    httpserver::Server server("test");

    EXPECT_THROW(server.start(std::filesystem::path("")), std::runtime_error);
}

TEST_F(ServerTest, StartInvalidSocketPath)
{
    httpserver::Server server("test");

    EXPECT_THROW(server.start(getSocketPath("invalid/test.sock")), std::runtime_error);
}

TEST_F(ServerTest, StartStop)
{
    httpserver::Server server("test");

    EXPECT_NO_THROW(server.start(getSocketPath("test.sock")));
    EXPECT_NO_THROW(server.stop());
}

TEST_F(ServerTest, StartStopCurrentThread)
{
    auto server = std::make_shared<httpserver::Server>("test");

    auto job = [server, path = getSocketPath("test.sock")]()
    {
        server->start(path, false);
    };

    std::thread t(job);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    server->stop();

    t.join();
}

TEST_F(ServerTest, StopNotStarted)
{
    httpserver::Server server("test");

    EXPECT_NO_THROW(server.stop());
}

TEST_F(ServerTest, StartAlreadyStarted)
{
    httpserver::Server server("test");

    EXPECT_NO_THROW(server.start(getSocketPath("test.sock")));
    EXPECT_THROW(server.start(getSocketPath("test.sock")), std::runtime_error);

    server.stop();
}
TEST_F(ServerTest, AddRoute)
{
    httpserver::Server server("test");
    auto fn = [&]()
    {
        server.addRoute(
            httpserver::Method::GET,
            "/test",
            [](const httplib::Request&, httplib::Response&) {}
        );
    };

    EXPECT_NO_THROW(fn());
}

TEST_F(ServerTest, AddOverrideRoute)
{
    httpserver::Server server("test");
    auto fn = [&]()
    {
        server.addRoute(
            httpserver::Method::GET,
            "/test",
            [](const httplib::Request&, httplib::Response&) {}
        );
    };

    EXPECT_NO_THROW(fn());
    EXPECT_NO_THROW(fn());
}
TEST_F(ServerTest, IsRunning)
{
    httpserver::Server server("test");

    EXPECT_FALSE(server.isRunning());
    EXPECT_NO_THROW(server.start(std::filesystem::path("/tmp/test.sock")));
    EXPECT_TRUE(server.isRunning());
    EXPECT_NO_THROW(server.stop());
    EXPECT_FALSE(server.isRunning());
}
