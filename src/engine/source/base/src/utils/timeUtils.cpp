#include "base/utils/stringUtils.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <mutex>

namespace base::utils::time {

namespace {

inline bool safeGmtime(const std::time_t& t, std::tm& out) {
#if defined(_WIN32)
    errno_t err = gmtime_s(&out, &t);
    return err == 0;
#elif defined(__unix__) || defined(__APPLE__)
    return gmtime_r(&t, &out) != nullptr;
#else
    std::tm* tmp = std::gmtime(&t);
    if (!tmp) return false;
    out = *tmp;
    return true;
#endif
}

inline bool safeLocaltime(const std::time_t& t, std::tm& out) {
#if defined(_WIN32)
    errno_t err = localtime_s(&out, &t);
    return err == 0;
#elif defined(__unix__) || defined(__APPLE__)
    return localtime_r(&t, &out) != nullptr;
#else
    std::tm* tmp = std::localtime(&t);
    if (!tmp) return false;
    out = *tmp;
    return true;
#endif
}


inline std::time_t timegm_portable(std::tm* tm) {
#if defined(_WIN32)
    // Windows has no timegm; hack: _mkgmtime is MSVC-specific
    return _mkgmtime(tm);
#elif defined(__unix__) || defined(__APPLE__)
    return timegm(tm);
#else
    // Fallback: you lose timezone correctness
    return std::mktime(tm);  // WARNING: assumes local time!
#endif
}

} // namespace

std::string getTimestamp(const std::time_t& time, const bool utc = true) {
    std::tm tm {};
    if (!(utc ? safeGmtime(time, tm) : safeLocaltime(time, tm)))
        return "";

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << (tm.tm_year + 1900) << '/'
       << std::setw(2) << (tm.tm_mon + 1) << '/'
       << std::setw(2) << tm.tm_mday << ' '
       << std::setw(2) << tm.tm_hour << ':'
       << std::setw(2) << tm.tm_min << ':'
       << std::setw(2) << tm.tm_sec;
    return ss.str();
}

std::string getCurrentTimestamp() {
    return getTimestamp(std::time(nullptr));
}

std::string getCurrentDate(const std::string& separator = "/") {
    auto date = base::utils::string::split(getCurrentTimestamp(), ' ').at(0);
    base::utils::string::replaceAll(date, "/", separator);
    return date;
}

std::string getCompactTimestamp(const std::time_t& time, const bool utc = true) {
    std::tm tm {};
    if (!(utc ? safeGmtime(time, tm) : safeLocaltime(time, tm)))
        return "";

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << (tm.tm_year + 1900)
       << std::setw(2) << (tm.tm_mon + 1)
       << std::setw(2) << tm.tm_mday
       << std::setw(2) << tm.tm_hour
       << std::setw(2) << tm.tm_min
       << std::setw(2) << tm.tm_sec;
    return ss.str();
}

std::string getCurrentISO8601() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    auto fractional = ms - sec;
    std::time_t tt = sec.count();

    std::tm tm {};
    if (!safeGmtime(tt, tm)) return "";

    std::ostringstream ss;
    ss << std::put_time(&tm, "%FT%T")
       << '.' << std::setfill('0') << std::setw(3) << fractional.count() << 'Z';
    return ss.str();
}

std::string timestampToISO8601(const std::string& timestamp) {
    std::tm tm {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y/%m/%d %H:%M:%S");
    if (ss.fail()) return "";

    std::time_t t = timegm_portable(&tm);
    if (t == -1) return "";

    auto now = std::chrono::system_clock::from_time_t(t);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    auto fractional = ms - sec;

    std::tm outTm {};
    if (!safeGmtime(t, outTm)) return "";

    std::ostringstream out;
    out << std::put_time(&outTm, "%FT%T")
        << '.' << std::setfill('0') << std::setw(3) << fractional.count() << 'Z';
    return out.str();
}

std::string rawTimestampToISO8601(const std::string& timestamp) {
    if (timestamp.empty() || !base::utils::string::isNumber(timestamp)) return "";

    std::time_t t;
    try {
        t = static_cast<std::time_t>(std::stoll(timestamp));
    } catch (...) {
        return "";
    }

    auto tp = std::chrono::system_clock::from_time_t(t);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    auto fractional = ms - sec;

    std::tm tm {};
    if (!safeGmtime(t, tm)) return "";

    std::ostringstream out;
    out << std::put_time(&tm, "%FT%T")
        << '.' << std::setfill('0') << std::setw(3) << fractional.count() << 'Z';
    return out.str();
}

std::chrono::seconds secondsSinceEpoch() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch());
}

int64_t getSecondsFromEpoch() {
    return secondsSinceEpoch().count();
}

} // namespace base::utils::time

