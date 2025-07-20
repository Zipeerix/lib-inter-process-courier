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

#include <InterProcessCourier/SyncServer.hpp>
#include "SyncUnixDomainServer.hpp"

namespace ipcourier {
static boost::asio::io_context k_ctx{}; // TODO: GET RID OF THIS ASAP! THIS IS JUST FOR TESTING PROBABLY TAKE AS ARG

SyncServer::SyncServer(const std::string& socket_addr) {
    m_server = std::make_unique<SyncUnixDomainServer>(k_ctx,
                                                      socket_addr,
                                                      [this](const ProtocolMessage& msg) {
                                                          // TODO: acceptMessage error handling should be expceton?
                                                          const auto accept_result = acceptMessage(msg);
                                                          if (!accept_result.has_value()) {
                                                              throw std::runtime_error(
                                                                  std::format(
                                                                      "Error while accepting message: {}:{}",
                                                                      static_cast<int>(accept_result.error().type),
                                                                      // todo: conv to string repr for above int
                                                                      accept_result.error().message));
                                                          }

                                                          return accept_result.value();
                                                      });
}

SyncServerResult<void> SyncServer::start() {
    const auto result = m_server->run();
    if (!result.has_value()) {
        // TODO: erorr mapping? its BAD here
        return std::unexpected(Error(SyncServerError::RuntimeError, result.error().message));
    }

    return {};
}

SyncServer::~SyncServer() {
}

SyncServerResult<SerializedProtoPayload> SyncServer::acceptMessage(const SerializedProtoPayload& serialized) {
    const std::unique_ptr<BaseProtoType> msg = tryParseProtobuf(serialized);
    if (!msg) {
        return std::unexpected(Error(SyncServerError::UnableToDeserializeMessage));
    }

    const auto type_name = msg->GetDescriptor()->full_name();
    const auto it = m_handlers.find(type_name);
    if (it == m_handlers.end()) {
        return std::unexpected(Error(SyncServerError::HandlerNotRegistered,
                                     std::format("No handler for {} registered", type_name)));
    }

    return it->second(*msg);
}

// TODO: Full rewrite or use ProtobufTools
std::unique_ptr<BaseProtoType> SyncServer::tryParseProtobuf(const SerializedProtoPayload& serialized) const {
    // TODO: I can use fn from ProtobufTools probably?
    std::string type_name;
    size_t sep = serialized.find(':');
    if (sep == std::string::npos) {
        return nullptr;
    }

    type_name = serialized.substr(0, sep);
    std::string data = serialized.substr(sep + 1);

    const auto* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (!descriptor) {
        return nullptr;
    }

    const BaseProtoType* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);

    if (!prototype) {
        return nullptr;
    }

    auto msg = std::unique_ptr<BaseProtoType>(prototype->New());
    if (!msg->ParseFromString(data)) {
        return nullptr;
    }

    return msg;
}
} // namespace ipcourier

