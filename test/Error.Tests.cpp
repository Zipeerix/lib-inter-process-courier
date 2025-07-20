/***************************************************************************
 *  InterProcessCourier Copyright (C) 2025  Ziperix                        *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <format>
#include <InterProcessCourier/Error.hpp>

enum class TestErrorType {
    None,
    FileNotFound,
    PermissionDenied,
    NetworkError,
    Unknown
};

template <>
struct std::formatter<TestErrorType> {
    static constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    static auto format(const TestErrorType type, std::format_context& ctx) {
        std::string_view name;
        switch (type) {
            case TestErrorType::None:
                name = "None";
                break;
            case TestErrorType::FileNotFound:
                name = "FileNotFound";
                break;
            case TestErrorType::PermissionDenied:
                name = "PermissionDenied";
                break;
            case TestErrorType::NetworkError:
                name = "NetworkError";
                break;
            case TestErrorType::Unknown:
                name = "Unknown";
                break;
            default:
                name = "InvalidTestErrorType";
                break;
        }

        return std::format_to(ctx.out(), "{}", name);
    }
};

TEST(Error, IsEnum_RecognizesEnumClass) {
    ASSERT_TRUE(ipcourier::IsEnum<TestErrorType>);
}

TEST(Error, IsEnum_RecognizesPlainEnum) {
    enum PlainEnum { Value1, Value2 };
    ASSERT_TRUE(ipcourier::IsEnum<PlainEnum>);
}

TEST(Error, IsEnum_RejectsNonEnumTypes) {
    ASSERT_FALSE(ipcourier::IsEnum<int>);
    ASSERT_FALSE(ipcourier::IsEnum<std::string>);
    ASSERT_FALSE(ipcourier::IsEnum<double>);
    struct MyStruct {
    };
    ASSERT_FALSE(ipcourier::IsEnum<MyStruct>);
}

TEST(Error, DefaultConstructor_InitializesCorrectly) {
    const auto error = ipcourier::Error<TestErrorType>();
    ASSERT_EQ(error.type, static_cast<TestErrorType>(0));
    ASSERT_TRUE(error.message.empty());
}

TEST(Error, Constructor_WithOnlyErrorType_InitializesCorrectly) {
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::FileNotFound);
    ASSERT_EQ(error.type, TestErrorType::FileNotFound);
    ASSERT_TRUE(error.message.empty());
}

TEST(Error, Constructor_WithErrorTypeAndMessage_InitializesCorrectly) {
    const std::string errorMessage = "File not found.";
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::FileNotFound, errorMessage);
    ASSERT_EQ(error.type, TestErrorType::FileNotFound);
    ASSERT_EQ(error.message, errorMessage);
}

TEST(Error, Constructor_WithErrorTypeAndEmptyMessage_InitializesCorrectly) {
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::PermissionDenied, "");
    ASSERT_EQ(error.type, TestErrorType::PermissionDenied);
    ASSERT_TRUE(error.message.empty());
}

TEST(Error, FormatsError_WithEmptyMessage) {
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::FileNotFound);
    const std::string formatted_string = std::format("{}", error);
    ASSERT_EQ(formatted_string, "Error: Type FileNotFound");
}

TEST(Error, FormatsError_WithNonEmptyMessage) {
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::NetworkError, "Host unreachable.");
    const std::string formatted_string = std::format("{}", error);
    ASSERT_EQ(formatted_string, "Error: Type NetworkError, Message: \"Host unreachable.\"");
}

TEST(Error, FormatsError_WithLongMessage) {
    std::string long_message = "This is a long error message.";
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::Unknown, long_message);
    const std::string formatted_string = std::format("{}", error);
    ASSERT_EQ(formatted_string, std::format("Error: Type Unknown, Message: \"{}\"", long_message));
}

TEST(Error, FormatsError_WithSpecialCharactersInMessage) {
    const auto error = ipcourier::Error<TestErrorType>(TestErrorType::PermissionDenied,
                                                       "Access denied to 'C:\\path\\file.ini'. Code: 0x5.");
    const std::string formatted_string = std::format("{}", error);
    ASSERT_EQ(formatted_string,
              "Error: Type PermissionDenied, Message: \"Access denied to 'C:\\path\\file.ini'. Code: 0x5.\"");
}

TEST(Error, FormatsError_WhenErrorTypeIsZeroValue) {
    const auto error = ipcourier::Error<TestErrorType>(static_cast<TestErrorType>(0), "Default error.");
    const std::string formatted_string = std::format("{}", error);
    ASSERT_EQ(formatted_string, "Error: Type None, Message: \"Default error.\"");
}
