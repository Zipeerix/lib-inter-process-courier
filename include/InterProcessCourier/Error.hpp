/***************************************************************************
 * InterProcessCourier Copyright (C) 2025  Ziperix                        *
 * *
 * This program is free software: you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                    *
 * *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 * *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * @file Error.hpp
 * @brief Utilities for error management.
 */

#ifndef INTER_PROCESS_COURIER_ERROR_HPP
#define INTER_PROCESS_COURIER_ERROR_HPP

#include <format>
#include <string>
#include <type_traits>

namespace ipcourier {
/**
 * @brief Concept to check if a type is an enumeration.
 *
 * This concept ensures that the `Error` struct template can only be
 * instantiated with an enumeration type for its `ErrorType` template
 * parameter. This design choice aligns with using enum classes for
 * specific error codes, providing strong typing for error categories.
 *
 * @tparam T The type to check.
 */
template <typename T>
concept IsEnum = std::is_enum_v<T>;

/**
 * @brief Represents a generic error structure within the InterProcessCourier library.
 *
 * This template struct is designed to provide a standardized way of
 * reporting errors, associating a specific error type (defined by an enum class)
 * with a human-readable message. It's intended to be used as the unexpected
 * value type within `std::expected` to convey detailed error information.
 *
 * @tparam ErrorType An enumeration type (e.g., an `enum class`) that defines
 * the specific categories or codes of errors.
 */
template <IsEnum ErrorType>
struct Error {
    /**
     * @brief The specific type or category of the error.
     *
     * This member holds an enum value that precisely identifies the
     * nature of the error, allowing for programmatic handling based
     * on predefined error conditions.
     */
    ErrorType type;

    /**
     * @brief A descriptive message providing more details about the error.
     *
     * This string can contain additional context, diagnostic information,
     * or a human-readable explanation of why the error occurred.
     * It complements the `type` field by offering more specific insights.
     */
    std::string message;

    /**
     * @brief Default constructor.
     *
     * Initializes `type` to its default-constructed value and `message` to an empty string.
     */
    Error() = default;

    /**
     * @brief Constructs an Error object with a specified error type and an optional message.
     * @param error_type The specific error type from the `ErrorType` enumeration.
     * @param error_message An optional string providing more details about the error.
     */
    explicit Error(ErrorType error_type, std::string error_message = "") :
        type(error_type), message(std::move(error_message)) {
    }
};
}  // namespace ipcourier

template <ipcourier::IsEnum ErrorType>
struct std::formatter<ipcourier::Error<ErrorType> > {
    static constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const ipcourier::Error<ErrorType>& error, std::format_context& ctx) const {
        const auto error_type_name = std::format("{}", error.type);
        if (error.message.empty()) {
            return std::format_to(ctx.out(), "Error: Type {}", error_type_name);
        }

        return std::format_to(ctx.out(), "Error: Type {}, Message: \"{}\"", error_type_name, error.message);
    }
};

#endif  // INTER_PROCESS_COURIER_ERROR_HPP
