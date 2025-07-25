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

#ifndef INTER_PROCESS_COURIER_SYNCUNIXDOMAINCLIENT_HPP
#define INTER_PROCESS_COURIER_SYNCUNIXDOMAINCLIENT_HPP

#include "UnixDomainProtocol.hpp"

#include <expected>

#include <InterProcessCourier/Error.hpp>
#include <boost/asio.hpp>

namespace ipcourier::_detail {
enum class UnixDomainClientError {
    UnknownError,
    ConnectionFailed,
    NotEnoughBytesReceived,
    UnableToSendMessage,
    UnableToReceiveMessage,
};

template <typename SuccessType>
using UnixDomainClientResult = std::expected<SuccessType, Error<UnixDomainClientError> >;

class SyncUnixDomainClient {
public:
    explicit SyncUnixDomainClient(boost::asio::io_context& io_context);

    UnixDomainClientResult<void> connect(const std::string& addr);

    void disconnect();

    UnixDomainClientResult<void> sendMessage(const ProtocolMessage& message);

    UnixDomainClientResult<ProtocolMessage> receiveMessage();

    UnixDomainClientResult<ProtocolMessage> sendAndReceiveMessage(const ProtocolMessage& message);

private:
    boost::asio::local::stream_protocol::socket m_socket;
};
}  // namespace ipcourier::_detail

#endif  // INTER_PROCESS_COURIER_SYNCUNIXDOMAINCLIENT_HPP
