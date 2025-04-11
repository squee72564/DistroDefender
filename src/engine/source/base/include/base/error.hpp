#ifndef _BASE_ERROR_HPP
#define _BASE_ERROR_HPP

#include <string>
#include <optional>
#include <variant>

namespace base
{

/**
 * @breif Error Struct
 *
 * This Error Struct is used to represent an error string for the Engine.
 * !note This struct is needed to disambiguate between a content string
 * and a string on variants.
 */
struct Error
{
    std::string message; ///< Error Message
};

using OptError = std::optional<Error>; ///< Optional Error

template <typename T>
using RespOrError = std::variant<T, Error>; ///< T or Error

/**
 * @brief Return No Error
 *
 * @return OptError
 */
inline OptError noError()
{
    return std::nullopt;
}

/**
 * @brief Check if an error is present in optional
 *
 * @param error
 * @return true if error is present
 * @return false if error is not present
 */
inline bool isError(const OptError& error)
{
    return error.has_value();
}

/**
 * @brief Check if a response is an error
 *
 * @tparam T Type of the response
 * @param respOrError
 * @return true if response is an error
 * @return false if response is not an error
 */
template <typename T>
inline bool isError(const RespOrError<T>& respOrError)
{
    return std::holds_alternative<Error>(respOrError);
}


/**
 * @brief Get the Response object
 *
 * @note This function should only be used if isError(respOrError) returns false
 *
 * @tparam T The Type of the Response
 * @param response
 * @return T
 *
 * @throws std::bad_variant_access if response is an error
 */
template <typename T>
inline auto getResponse(const RespOrError<T>& response) -> decltype(std::get<T>(response))
{
    return std::get<T>(response);
}

template <typename T>
inline auto getResponse(RespOrError<T>& response) -> decltype(std::get<T>(response))
{
    return std::get<T>(response);
} 

template <typename T>
inline auto getResponse(RespOrError<T>&& response) -> decltype(std::get<T>(std::move(response)))
{
    return std::get<T>(std::move(response));
} 

/**
 * @brief Get the Error object
 *
 * @note This function should only be used if isError(respOrError) returns true
 *
 * @tparam T The Type of the Response
 * @param error
 * @return Error
 *
 * @throws std::bad_variant_access if response is not an error
 */
template <typename T>
inline auto getError(const RespOrError<T>& error) -> decltype(std::get<Error>(error))
{
    return std::get<Error>(error);
}

template <typename T>
inline auto getError(RespOrError<T>& error) -> decltype(std::get<Error>(error))
{
    return std::get<Error>(error);
}

template <typename T>
inline auto getError(RespOrError<T>&& error) -> decltype(std::get<Error>(error))
{
    return std::get<Error>(std::move(error));
}

inline Error& getError(OptError& error)
{
    return error.value();
}

inline const Error& getError(const OptError& error)
{
    return error.value();
}

} // namespace base

#endif // _BASE_ERROR_HPP
