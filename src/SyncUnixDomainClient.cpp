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

namespace ipcourier::_detail {
SyncUnixDomainClient::SyncUnixDomainClient(boost::asio::io_context& io_context) : m_socket(io_context) {
}

UnixDomainClientResult<void> SyncUnixDomainClient::connect(const std::string& addr) {
    try {
        m_socket.connect(boost::asio::local::stream_protocol::endpoint(addr));
    } catch (const std::exception& e) {
        return std::unexpected(Error(UnixDomainClientError::ConnectionFailed, e.what()));
    }

    return {};
}

void SyncUnixDomainClient::disconnect() {
    m_socket.close();
}

UnixDomainClientResult<void> SyncUnixDomainClient::sendMessage(const ProtocolMessage& message) {
    const auto payload_length = static_cast<std::uint32_t>(message.length());
    ProtocolMessageBuffer payload_buffer;
    payload_buffer.resize(k_payload_length_header_size + payload_length);

    std::memcpy(payload_buffer.data(), &payload_length, k_payload_length_header_size);
    std::memcpy(payload_buffer.data() + k_payload_length_header_size, message.data(), message.length());

    try {
        boost::asio::write(m_socket, boost::asio::buffer(payload_buffer));
    } catch (std::exception& e) {
        return std::unexpected(Error(UnixDomainClientError::UnableToSendMessage, e.what()));
    }

    return {};
}

UnixDomainClientResult<ProtocolMessage> SyncUnixDomainClient::receiveMessage() {
    try {
        ProtocolMessageBuffer message_length_header_reply_buffer;
        message_length_header_reply_buffer.resize(k_payload_length_header_size);

        auto reply_length =
            m_socket.read_some(boost::asio::buffer(message_length_header_reply_buffer, k_payload_length_header_size));
        if (reply_length != k_payload_length_header_size) {
            return std::unexpected(Error(UnixDomainClientError::NotEnoughBytesReceived));
        }

        std::uint32_t message_length = 0;
        std::memcpy(&message_length, message_length_header_reply_buffer.data(), k_payload_length_header_size);

        ProtocolMessageBuffer reply_buffer;
        reply_buffer.resize(message_length);

        reply_length = m_socket.read_some(boost::asio::buffer(reply_buffer.data(), message_length));
        if (reply_length != message_length) {
            return std::unexpected(Error(UnixDomainClientError::NotEnoughBytesReceived));
        }

        return ProtocolMessage(reply_buffer.begin(), reply_buffer.end());
        ;
    } catch (std::exception& e) {
        return std::unexpected(Error(UnixDomainClientError::UnableToReceiveMessage, e.what()));
    }
}

UnixDomainClientResult<ProtocolMessage> SyncUnixDomainClient::sendAndReceiveMessage(const ProtocolMessage& message) {
    const auto send_result = sendMessage(message);
    if (!send_result.has_value()) {
        return std::unexpected(send_result.error());
    }

    return receiveMessage();
}
}  // namespace ipcourier::_detail
