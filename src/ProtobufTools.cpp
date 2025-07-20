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

#include <format>

#include <InterProcessCourier/ProtobufTools.hpp>

namespace ipcourier {
SerializedProtoPayload createProtoPayload(const std::string_view type_name, const std::string_view serialized_data) {
    return std::format("{}:{}", type_name, serialized_data);
}

ProtobufToolResult<std::unique_ptr<BaseProtoType> > makeBaseProtoFromPayload(const SerializedProtoPayload& payload) {
    const auto delimiter_pos = payload.find(':');
    if (delimiter_pos == std::string::npos) {
        const auto message = payload.size() > 128 ? payload.substr(0, 128) + "..." : payload;
        return std::unexpected(Error(ProtoPayloadParseError::InvalidFormat,
                                     std::format("Received message: {}",
                                                 message)));
    }

    const auto type_name = payload.substr(0, delimiter_pos);
    const auto serialized_data = payload.substr(delimiter_pos + 1);

    const auto* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (descriptor == nullptr) {
        return std::unexpected(Error(ProtoPayloadParseError::DeserializationFailed,
                                     std::format("Description for {} not found", type_name)));
    }

    const BaseProtoType* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
    if (prototype == nullptr) {
        return std::unexpected(Error(ProtoPayloadParseError::DeserializationFailed,
                                     std::format("Unable to generate prototype for {}", type_name)));
    }

    auto msg = std::unique_ptr<BaseProtoType>(prototype->New());
    if (!msg->ParseFromString(serialized_data)) {
        const auto message = payload.size() > 128 ? payload.substr(0, 128) + "..." : payload;
        return std::unexpected(Error(ProtoPayloadParseError::DeserializationFailed,
                                     std::format("Unable to deserialize as {}. Message: {}", type_name, message)));
    }

    return std::move(msg);
}
} // namespace ipcourier
