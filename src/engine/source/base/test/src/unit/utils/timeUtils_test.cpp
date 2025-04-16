#include "gtest/gtest.h"
#include <base/utils/timeUtils.hpp>
#include <base/utils/stringUtils.hpp>
#include <regex>
#include <thread>
#include <iomanip>

using namespace base::utils::time;

class TimeUtilsTest : public ::testing::Test {
protected:
    TimeUtilsTest() = default;
    ~TimeUtilsTest() override = default;
};

TEST_F(TimeUtilsTest, GetTimestampUTC)
{
    std::time_t now = 0; // Epoch
    std::string result = getTimestamp(now, true);
    EXPECT_EQ(result, "1970/01/01 00:00:00");
}

TEST_F(TimeUtilsTest, GetTimestampLocal)
{
    std::time_t now = 0;
    std::string result = getTimestamp(now, false);
    std::tm* local = std::localtime(&now);
    std::ostringstream ss;
    ss << std::put_time(local, "%Y/%m/%d %H:%M:%S");
    EXPECT_EQ(result, ss.str());
}

TEST_F(TimeUtilsTest, GetCurrentTimestampFormat)
{
    std::string ts = getCurrentTimestamp();
    std::regex pattern(R"(\d{4}/\d{2}/\d{2} \d{2}:\d{2}:\d{2})");
    EXPECT_TRUE(std::regex_match(ts, pattern));
}

TEST_F(TimeUtilsTest, GetCurrentDateDefaultSeparator)
{
    std::string date = getCurrentDate();
    std::regex pattern(R"(\d{4}/\d{2}/\d{2})");
    EXPECT_TRUE(std::regex_match(date, pattern));
}

TEST_F(TimeUtilsTest, GetCurrentDateCustomSeparator)
{
    std::string date = getCurrentDate("-");
    std::regex pattern(R"(\d{4}-\d{2}-\d{2})");
    EXPECT_TRUE(std::regex_match(date, pattern));
}

TEST_F(TimeUtilsTest, GetCompactTimestampFormat)
{
    std::time_t now = 0;
    std::string result = getCompactTimestamp(now, true);
    EXPECT_EQ(result, "19700101000000");
}

TEST_F(TimeUtilsTest, GetCompactTimestampLocal)
{
    std::time_t now = 0;
    std::string result = getCompactTimestamp(now, false);
    std::tm* local = std::localtime(&now);
    std::ostringstream ss;
    ss << std::put_time(local, "%Y%m%d%H%M%S");
    EXPECT_EQ(result, ss.str());
}

TEST_F(TimeUtilsTest, GetCurrentISO8601Format)
{
    std::string iso = getCurrentISO8601();
    std::regex pattern(R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z)");
    EXPECT_TRUE(std::regex_match(iso, pattern));
}

TEST_F(TimeUtilsTest, TimestampToISO8601Valid)
{
    std::string input = "1970/01/01 00:00:00";
    std::string iso = timestampToISO8601(input);
    std::regex pattern(R"(1970-01-01T00:00:00\.000Z)");
    ASSERT_TRUE(std::regex_match(iso, pattern));
}

TEST_F(TimeUtilsTest, TimestampToISO8601Invalid)
{
    std::string input = "invalid date";
    EXPECT_EQ(timestampToISO8601(input), "");
}

TEST_F(TimeUtilsTest, RawTimestampToISO8601Valid)
{
    std::string input = "0";
    std::string iso = rawTimestampToISO8601(input);
    std::regex pattern(R"(1970-01-01T00:00:00\.000Z)");
    EXPECT_TRUE(std::regex_match(iso, pattern));
}

TEST_F(TimeUtilsTest, RawTimestampToISO8601InvalidNonNumeric)
{
    std::string input = "abc";
    EXPECT_EQ(rawTimestampToISO8601(input), "");
}

TEST_F(TimeUtilsTest, RawTimestampToISO8601Empty)
{
    EXPECT_EQ(rawTimestampToISO8601(""), "");
}

TEST_F(TimeUtilsTest, SecondsSinceEpochIncreasing)
{
    auto t1 = secondsSinceEpoch();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto t2 = secondsSinceEpoch();
    EXPECT_GT(t2.count(), t1.count());
}

TEST_F(TimeUtilsTest, GetSecondsFromEpochIncreasing)
{
    auto t1 = getSecondsFromEpoch();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto t2 = getSecondsFromEpoch();
    EXPECT_GT(t2, t1);
}

