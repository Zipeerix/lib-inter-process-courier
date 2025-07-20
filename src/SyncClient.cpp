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

#include <InterProcessCourier/SyncClient.hpp>
#include "SyncUnixDomainClient.hpp"

namespace ipcourier {
static boost::asio::io_context k_ctx{}; // TODO: GET RID OF THIS ASAP! THIS IS JUST FOR TESTING PROBABLY TAKE AS ARG

SyncClient::SyncClient(const std::string& socket_addr) : m_socket_addr(socket_addr) {
    m_client = std::make_unique<SyncUnixDomainClient>(k_ctx);
}

SyncClient::~SyncClient() {
}

SyncClientResult<void> SyncClient::connect() const {
    const auto connect_result = m_client->connect(m_socket_addr);
    if (!connect_result.has_value()) {
        return std::unexpected(Error(SyncClientError::UnableToConnectToServer, connect_result.error().message));
    }

    return {};
}


SyncClientResult<std::string> SyncClient::sendAndReceiveMessage(const SerializedProtoPayload& serialized) const {
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
} // namespace ipcourier
