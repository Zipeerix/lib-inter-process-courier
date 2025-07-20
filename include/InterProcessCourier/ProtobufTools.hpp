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

#ifndef INTER_PROCESS_COURIER_PROTOBUF_TOOLS_HPP
#define INTER_PROCESS_COURIER_PROTOBUF_TOOLS_HPP

#include <expected>
#include <string>
#include <string_view>

#include <google/protobuf/message.h>
#include <google/protobuf/empty.pb.h>

#include <InterProcessCourier/Error.hpp>


/**
 * @file ProtobufTools.hpp
 * @brief Utilities for serializing and deserializing Protocol Buffer messages to/from payload strings.
 */

namespace ipcourier {
/**
 * @brief Error codes for Protocol Buffer payload parsing operations.
 */
enum class ProtoPayloadParseError {
    InvalidFormat,        ///< Missing or malformed delimiter in payload string
    TypeMismatch,         ///< Type name in the payload doesn't match an expected Protocol Buffer type
    DeserializationFailed ///< Protocol Buffer parsing failed (corrupt data, wrong format, etc.)
};

/**
 * @brief Type alias for the result of Protocol Buffer tool operations.
 *
 * This alias represents an `std::expected` type where success is indicated by `SuccessType`
 * and failure by an `Error` object containing a `ProtoPayloadParseError`.
 *
 * @tparam SuccessType The type returned on successful operation.
 */
template <typename SuccessType>
using ProtobufToolResult = std::expected<SuccessType, Error<ProtoPayloadParseError> >;

/// Type alias for a base type of all Protocol Buffer messages
using BaseProtoType = google::protobuf::Message;

/// Type alias for an empty protobuf message
using NoMessage = google::protobuf::Empty;

/**
 * @brief Concept to ensure template types derive from the Protocol Buffer Message base class.
 * @tparam T The type to check
 */
template <typename T>
concept IsDerivedFromProtoMessage = std::derived_from<T, BaseProtoType>;

/// Type alias for serialized protocol buffer payload
using SerializedProtoPayload = std::string;

/**
 * @brief Creates a formatted payload string from type name and serialized data.
 *
 * Creates a payload in the format "type_name:serialized_data" where the colon
 * serves as a delimiter between the Protocol Buffer type identifier and the
 * actual serialized message data.
 *
 * @param type_name The full name of the Protocol Buffer message type
 * @param serialized_data The serialized Protocol Buffer message data
 * @return SerializedProtoPayload The formatted payload string
 */
SerializedProtoPayload createProtoPayload(std::string_view type_name, std::string_view serialized_data);

/**
 * @brief Serializes a Protocol Buffer message into a payload string.
 *
 * Takes a Protocol Buffer message, serializes it to binary format, and creates
 * a payload string that includes both the message type information and the
 * serialized data. The resulting payload can be transmitted over inter-process
 * communication channels and later deserialized back to the original message type.
 *
 * @tparam ProtoType The Protocol Buffer message type (must derive from google::protobuf::Message)
 * @param message The Protocol Buffer message to serialize
 * @return std::string The formatted payload string containing type info and serialized data
 *
 * @see makeProtoFromPayload for the reverse operation
 */
template <IsDerivedFromProtoMessage ProtoType>
SerializedProtoPayload makePayloadFromProto(const ProtoType& message) {
    std::string serialized_data;
    message.SerializeToString(&serialized_data);

    return createProtoPayload(ProtoType::descriptor()->full_name(), serialized_data);
}


/**
 * @brief Deserializes a payload string back into a Protocol Buffer message.
 * * Parses a payload string created by makePayloadFromProto() and reconstructs
 * the original Protocol Buffer message. The function validates that the type
 * information in the payload matches the expected message type before attempting
 * deserialization.
 *
 * @tparam ProtoType The expected Protocol Buffer message type (must derive from Message)
 * @param payload The payload string containing type info and serialized data
 * @return ProtobufToolResult<ProtoType> Either the deserialized message or an error code
 * indicating the failure reason
 * * @retval ProtoType Successfully deserialized Protocol Buffer message
 * @retval InvalidFormat The payload string doesn't contain the expected delimiter
 * @retval TypeMismatch The type name in the payload doesn't match ProtoType
 * @retval DeserializationFailed The Protocol Buffer parsing failed
 *
 * @see makePayloadFromProto for the reverse operation
 *
 * @par Example:
 * @code
 * // Assuming MyMessage is a defined Protocol Buffer message
 * const SerializedProtoPayload payload = "my.package.MyMessage:some_binary_data";
 * const auto result = ipcourier::makeProtoFromPayload<MyMessage>(payload);
 * if (result) {
 * MyMessage msg = *result;
 * // Use msg
 * } else {
 * // Handle result.error()
 * }
 * @endcode
 */
template <IsDerivedFromProtoMessage ProtoType>
ProtobufToolResult<ProtoType> makeProtoFromPayload(const SerializedProtoPayload& payload) {
    const auto delimiter_pos = payload.find(':');
    if (delimiter_pos == std::string::npos) {
        const auto message = payload.size() > 128 ? payload.substr(0, 128) + "..." : payload;
        return std::unexpected(Error(ProtoPayloadParseError::InvalidFormat,
                                     std::format("Received message: {}",
                                                 message)));
    }

    const auto type_name = payload.substr(0, delimiter_pos);
    const auto serialized_data = payload.substr(delimiter_pos + 1);

    if (type_name != ProtoType::descriptor()->full_name()) {
        return std::unexpected(Error(ProtoPayloadParseError::TypeMismatch,
                                     std::format("Received {} but expected {}",
                                                 type_name,
                                                 ProtoType::descriptor()->full_name())));
    }

    ProtoType message;
    if (!message.ParseFromString(std::string(serialized_data))) {
        return std::unexpected(Error(ProtoPayloadParseError::DeserializationFailed));
    }

    return message;
}
} // namespace ipcourier

#endif  // INTER_PROCESS_COURIER_PROTOBUF_TOOLS_HPP
