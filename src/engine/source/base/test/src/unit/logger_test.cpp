#include <gtest/gtest.h>
#include <filesystem>
#include <iostream>
#include <string>

#include <stdlib.h>

#include <base/logger.hpp>

class LoggerTest: public ::testing::Test
{
public:
    std::string m_tmpPath;

    void SetUp() override
    {
        char tempFileName[] = "/tmp/temp_log_XXXXXX";
        auto tempFileDescriptor = mkstemp(tempFileName);
        m_tmpPath = tempFileName;
        ASSERT_NE(tempFileDescriptor, -1);
    }

    void TearDown() override
    {
        logger::stop();
        std::filesystem::remove(m_tmpPath);
    }
};

TEST_F(LoggerTest, LogNonExist)
{
    ASSERT_ANY_THROW(logger::setLevel(logger::LogLevel::Info));
}

TEST_F(LoggerTest, LogSuccessStart)
{
    ASSERT_NO_THROW(logger::start(logger::LoggerConfig {.filePath = m_tmpPath, .level = logger::LogLevel::Info}));
}

TEST_F(LoggerTest, LogRepeatedStart)
{
    ASSERT_NO_THROW(logger::start(logger::LoggerConfig {.filePath = m_tmpPath}));
    ASSERT_ANY_THROW(logger::start(logger::LoggerConfig {.filePath = m_tmpPath}));
}

TEST_F(LoggerTest, LogGetSomeInstance)
{
    ASSERT_NO_THROW(logger::start(logger::LoggerConfig {.filePath = m_tmpPath}));
    auto logger = logger::getDefaultLogger();
    auto someLogger = logger::getDefaultLogger();
    ASSERT_NE(logger, nullptr);
    ASSERT_EQ(logger, someLogger);
}
