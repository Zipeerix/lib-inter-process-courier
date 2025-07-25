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

#include "SyncUnixDomainClient.hpp"

#include <format>
#include <stdexcept>

#include <InterProcessCourier/SyncClient.hpp>
#include <InterProcessCourier/detail/DuplicateRegistrationHandler.hpp>
#include <boost/asio.hpp>

#include "InternalRequests.pb.h"

namespace ipcourier {
SyncClient::SyncClient(std::string socket_addr, SyncClientOptions client_options) :
    m_client_options(std::move(client_options)), m_socket_addr(std::move(socket_addr)),
    m_io_context(std::make_unique<boost::asio::io_context>()),
    m_client(std::make_unique<_detail::SyncUnixDomainClient>(*m_io_context)) {
}

SyncClient::~SyncClient() = default;

SyncClientResult<void> SyncClient::connect() {
    const auto connect_result = m_client->connect(m_socket_addr);
    if (!connect_result.has_value()) {
        return std::unexpected(Error(SyncClientError::UnableToConnectToServer, connect_result.error().message));
    }

    if (m_client_options.validate_req_res_pair_strategy == ValidateRequestResponsePairStrategy::ServerReflection) {
        const auto reflect_result = reflectRequestResponseMappingPairs();
        if (!reflect_result.has_value()) {
            return std::unexpected(reflect_result.error());
        }
    }

    return {};
}

SyncClientResult<void> SyncClient::reflectRequestResponseMappingPairs() {
    using MappingReflectionRequest = internal_request_proto::IPCInternal_GetRequestResponseMappingPairsRequest;
    using MappingReflectionResponse = internal_request_proto::IPCInternal_GetRequestResponseMappingPairsResponse;

    registerRequestResponsePair<MappingReflectionRequest, MappingReflectionResponse>();

    const auto mapping_reflect_result =
        sendRequest<MappingReflectionRequest, MappingReflectionResponse>(MappingReflectionRequest{});
    if (!mapping_reflect_result.has_value()) {
        return std::unexpected(Error(SyncClientError::UnableToReflectMappings, mapping_reflect_result.error().message));
    }

    const auto& mapping_reflect_response = mapping_reflect_result.value();
    for (const auto& [key, value] : mapping_reflect_response.mappings()) {
        m_request_response_pairs[key] = value;
    }

    return {};
}

bool SyncClient::registerDuplicateRequestResponsePair(const std::string& request_name,
                                                      const std::string& response_name) {
    const auto register_handler = [this](const auto& handler_request_name, const auto& handler_response_name) {
        this->registerValidatedRequestResponsePair(handler_request_name, handler_response_name);
    };
    return _detail::registerDuplicateRequestResponsePair(
        m_client_options.duplicate_registration_strategy, register_handler, request_name, response_name);
}

void SyncClient::registerValidatedRequestResponsePair(const std::string& request_name,
                                                      const std::string& response_name) {
    m_request_response_pairs[request_name] = response_name;
}

SyncClientResult<_detail::SerializedProtoPayload> SyncClient::sendAndReceiveMessage(
    const _detail::SerializedProtoPayload& serialized) const {
    const auto send_result = m_client->sendMessage(serialized);
    if (!send_result.has_value()) {
        return std::unexpected(Error(SyncClientError::UnableToSendMessage, send_result.error().message));
    }

    const auto receive_result = m_client->receiveMessage();
    if (!receive_result.has_value()) {
        return std::unexpected(Error(SyncClientError::UnableToReceiveMessage, receive_result.error().message));
    }

    return receive_result.value();
}
}  // namespace ipcourier
