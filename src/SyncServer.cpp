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
#include <boost/asio.hpp>
#include "InternalRequests.pb.h"
#include "SyncUnixDomainServer.hpp"

namespace ipcourier {
std::string convertSyncServerErrorToString(const SyncServerError error_type) {
    switch (error_type) {
        case SyncServerError::UnknownError:
            return "Unknown error";
        case SyncServerError::HandlerNotRegistered:
            return "Handler not registered";
        case SyncServerError::RuntimeError:
            return "Runtime error";
        case SyncServerError::UnableToDeserializeMessage:
            return "Unable to deserialize message";
        case SyncServerError::UnableToSerializeMessage:
            return "Unable to serialize message";
    }

    throw std::logic_error("Invalid SyncServerError given for conversion to string");
}

SyncServer::SyncServer(boost::asio::io_context& io_context, const std::string& socket_addr) {
    m_server = std::make_unique<SyncUnixDomainServer>(io_context,
                                                      socket_addr,
                                                      [this](const ProtocolMessage& msg) {
                                                          // TODO: acceptMessage error handling should be expceton?
                                                          const auto accept_result = acceptMessage(msg);
                                                          if (!accept_result.has_value()) {
                                                              throw std::runtime_error(
                                                                  std::format(
                                                                      "Error while accepting message: {}",
                                                                      accept_result.error())
                                                                  );
                                                          }

                                                          return accept_result.value();
                                                      });
    registerHandler<internal_request_proto::GetRequestResponseMappingPairsRequest,
                    internal_request_proto::GetRequestResponseMappingPairsResponse>([this](const auto&) {
        internal_request_proto::GetRequestResponseMappingPairsResponse response;

        auto* proto_map = response.mutable_mappings();
        for (const auto& [request_name, response_name] : m_request_response_pairs) {
            proto_map->emplace(request_name, response_name);
        }

        return response;
    });
}

SyncServerResult<void> SyncServer::start() {
    const auto result = m_server->run();
    if (!result.has_value()) {
        // TODO: erorr mapping? Remove runtime error altogether maybe
        return std::unexpected(Error(SyncServerError::RuntimeError, result.error().message));
    }

    return {};
}

SyncServer::~SyncServer() = default;

SyncServerResult<SerializedProtoPayload> SyncServer::acceptMessage(const SerializedProtoPayload& serialized) {
    const auto msg_result = makeBaseProtoFromPayload(serialized);
    if (!msg_result.has_value()) {
        return std::unexpected(Error(SyncServerError::UnableToDeserializeMessage, msg_result.error().message));
    }

    const auto& msg = msg_result.value();

    const auto type_name = msg->GetDescriptor()->full_name();
    const auto it = m_handlers.find(type_name);
    if (it == m_handlers.end()) {
        return std::unexpected(Error(SyncServerError::HandlerNotRegistered,
                                     std::format("No handler for {} registered", type_name)));
    }

    return it->second(*msg);
}
} // namespace ipcourier

