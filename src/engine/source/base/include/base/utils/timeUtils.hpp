#ifndef _BASE_UTILS_TIME_UTILS_HPP
#define _BASE_UTILS_TIME_UTILS_HPP

#include <chrono>
#include <string>

namespace base::utils::time {

/**
 * @brief Format a given time as a human-readable timestamp.
 * @param time Time to format.
 * @param utc If true, formats as UTC time; otherwise, local time.
 * @return Formatted string in "YYYY/MM/DD hh:mm:ss" format.
 */
std::string getTimestamp(const std::time_t& time, bool utc = true);

/**
 * @brief Get the current time as a formatted timestamp.
 * @return Current time in "YYYY/MM/DD hh:mm:ss" format (UTC).
 */
std::string getCurrentTimestamp();

/**
 * @brief Get the current date as a string.
 * @param separator Character(s) to use between year, month, and day.
 * @return Date string in "YYYY<sep>MM<sep>DD" format (UTC).
 */
std::string getCurrentDate(const std::string& separator = "/");

/**
 * @brief Format a time as a compact timestamp without separators.
 * @param time Time to format.
 * @param utc If true, formats as UTC time; otherwise, local time.
 * @return Compact timestamp in "YYYYMMDDhhmmss" format.
 */
std::string getCompactTimestamp(const std::time_t& time, bool utc = true);

/**
 * @brief Get the current UTC time in ISO 8601 format with milliseconds.
 * @return ISO 8601 string in "YYYY-MM-DDThh:mm:ss.sssZ" format.
 */
std::string getCurrentISO8601();

/**
 * @brief Convert a timestamp string to ISO 8601 format with milliseconds.
 * @param timestamp Timestamp string in "YYYY/MM/DD hh:mm:ss" format.
 * @return ISO 8601 formatted string, or empty string on parse failure.
 */
std::string timestampToISO8601(const std::string& timestamp);

/**
 * @brief Convert a raw epoch timestamp string to ISO 8601 format.
 * @param timestamp Epoch time as string (in seconds).
 * @return ISO 8601 formatted string, or empty string on parse failure.
 */
std::string rawTimestampToISO8601(const std::string& timestamp);

/**
 * @brief Get the number of seconds since the Unix epoch.
 * @return Duration in seconds since 1970-01-01T00:00:00Z.
 */
std::chrono::seconds secondsSinceEpoch();

/**
 * @brief Get the number of seconds since the Unix epoch as an integer.
 * @return Seconds since 1970-01-01T00:00:00Z.
 */
int64_t getSecondsFromEpoch();

} // namespace base::utils::time

#endif // _BASE_UTILS_TIME_UTILS_HPP
