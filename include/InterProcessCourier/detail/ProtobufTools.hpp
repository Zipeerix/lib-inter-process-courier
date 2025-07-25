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

#include <InterProcessCourier/Error.hpp>
#include <InterProcessCourier/ProtobufInterface.hpp>
#include <google/protobuf/empty.pb.h>
#include <google/protobuf/message.h>

namespace ipcourier::_detail {
using SerializedProtoPayload = std::string;

enum class ProtoPayloadParseError {
    InvalidFormat,
    TypeMismatch,
    DeserializationFailed
};

template <typename SuccessType>
using ProtobufToolResult = std::expected<SuccessType, Error<ProtoPayloadParseError> >;

SerializedProtoPayload createProtoPayload(std::string_view type_name, std::string_view serialized_data);

template <IsDerivedFromProtoMessage ProtoType>
SerializedProtoPayload makePayloadFromProto(const ProtoType& message) {
    std::string serialized_data;
    message.SerializeToString(&serialized_data);

    return createProtoPayload(ProtoType::descriptor()->full_name(), serialized_data);
}

template <IsDerivedFromProtoMessage ProtoType>
ProtobufToolResult<ProtoType> makeProtoFromPayload(const SerializedProtoPayload& payload) {
    const auto delimiter_pos = payload.find(':');
    if (delimiter_pos == std::string::npos) {
        const auto message = payload.size() > 128 ? payload.substr(0, 128) + "..." : payload;
        return std::unexpected(
            Error(ProtoPayloadParseError::InvalidFormat, std::format("Received message: {}", message)));
    }

    const auto type_name = payload.substr(0, delimiter_pos);
    const auto serialized_data = payload.substr(delimiter_pos + 1);

    if (type_name != ProtoType::descriptor()->full_name()) {
        return std::unexpected(
            Error(ProtoPayloadParseError::TypeMismatch,
                  std::format("Received {} but expected {}", type_name, ProtoType::descriptor()->full_name())));
    }

    ProtoType message;
    if (!message.ParseFromString(std::string(serialized_data))) {
        return std::unexpected(Error(ProtoPayloadParseError::DeserializationFailed));
    }

    return message;
}

ProtobufToolResult<std::unique_ptr<BaseProtoType> > makeBaseProtoFromPayload(const SerializedProtoPayload& payload);
}  // namespace ipcourier::_detail

#endif  // INTER_PROCESS_COURIER_PROTOBUF_TOOLS_HPP
