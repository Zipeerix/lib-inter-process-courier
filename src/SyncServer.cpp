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

#include "SyncUnixDomainServer.hpp"

#include <InterProcessCourier/SyncServer.hpp>
#include <boost/asio.hpp>

#include "InternalRequests.pb.h"

namespace ipcourier {
SyncServer::SyncServer(std::string socket_addr, SyncServerOptions server_options) :
    m_server_options(std::move(server_options)), m_socket_addr(std::move(socket_addr)),
    m_io_context(std::make_unique<boost::asio::io_context>()) {
    m_server = std::make_unique<_detail::SyncUnixDomainServer>(
        *m_io_context, m_socket_addr, [this](const _detail::ProtocolMessage& msg) {
            // TODO: acceptMessage error handling should be exception?
            const auto accept_result = acceptMessage(msg);
            if (!accept_result.has_value()) {
                throw std::runtime_error(std::format("Error while accepting message: {}", accept_result.error()));
            }

            return accept_result.value();
        });

    using MappingReflectionRequest = internal_request_proto::IPCInternal_GetRequestResponseMappingPairsRequest;
    using MappingReflectionResponse = internal_request_proto::IPCInternal_GetRequestResponseMappingPairsResponse;

    registerHandler<MappingReflectionRequest, MappingReflectionResponse>([this](const auto&) {
        MappingReflectionResponse response;

        auto* proto_map = response.mutable_mappings();
        for (const auto& [request_name, response_name] : m_request_response_pairs) {
            proto_map->emplace(request_name, response_name);
        }

        return response;
    });
}

SyncServerResult<void> SyncServer::start() const {
    const auto result = m_server->run();
    if (!result.has_value()) {
        // TODO: erorr mapping? Remove runtime error altogether maybe
        return std::unexpected(Error(SyncServerError::RuntimeError, result.error().message));
    }

    return {};
}

SyncServer::~SyncServer() = default;

SyncServerResult<_detail::SerializedProtoPayload> SyncServer::acceptMessage(
    const _detail::SerializedProtoPayload& serialized) {
    const auto msg_result = _detail::makeBaseProtoFromPayload(serialized);
    if (!msg_result.has_value()) {
        return std::unexpected(Error(SyncServerError::UnableToDeserializeMessage, msg_result.error().message));
    }

    const auto& msg = msg_result.value();

    const auto type_name = msg->GetDescriptor()->full_name();
    const auto it = m_handlers.find(type_name);
    if (it == m_handlers.end()) {
        return std::unexpected(
            Error(SyncServerError::HandlerNotRegistered, std::format("No handler for {} registered", type_name)));
    }

    return it->second(*msg);
}
}  // namespace ipcourier
