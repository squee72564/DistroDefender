#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

#include <base/logger.hpp>
#include <httpserver/server.hpp>

#include <conf/confLoader.hpp>

TEST(ConfLoader, testLoader)
{
    logger::testInit();
    std::shared_ptr<conf::IConfLoader> confLoader = std::make_shared<conf::ConfLoader>();
    json::Json cnf {};

    // Test
    EXPECT_NO_THROW(cnf = (*confLoader)());
}
