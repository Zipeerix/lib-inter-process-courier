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

#ifndef INTER_PROCESS_COURIER_SYNCUNIXDOMAINSERVER_HPP
#define INTER_PROCESS_COURIER_SYNCUNIXDOMAINSERVER_HPP

#include "UnixDomainProtocol.hpp"

#include <expected>
#include <functional>
#include <string>
#include <vector>

#include <InterProcessCourier/Error.hpp>
#include <boost/asio.hpp>

namespace ipcourier::_detail {
enum class UnixDomainServerError {
    UnknownError,
    NotEnoughBytesReceived,
    GeneralServerError,
    GeneralServerSessionError,
    UnableToSendMessage
};

using RequestHandler = std::function<ProtocolMessage(const ProtocolMessage&)>;

template <typename SuccessType>
using UnixDomainServerResult = std::expected<SuccessType, Error<UnixDomainServerError> >;

class SyncUnixDomainSession {
public:
    SyncUnixDomainSession(boost::asio::local::stream_protocol::socket socket, RequestHandler request_handler);

    UnixDomainServerResult<void> start();

private:
    boost::asio::local::stream_protocol::socket m_socket;
    RequestHandler m_request_handler;

    UnixDomainServerResult<std::uint32_t> readHeader();

    UnixDomainServerResult<ProtocolMessageBuffer> readBody(std::uint32_t msg_length);

    UnixDomainServerResult<void> writeResponse(const ProtocolMessageBuffer& response);
};

class SyncUnixDomainServer {
public:
    SyncUnixDomainServer(boost::asio::io_context& io_context,
                         const std::string& socket_path,
                         RequestHandler request_handler);

    UnixDomainServerResult<void> run();

private:
    boost::asio::io_context& m_io_context;
    boost::asio::local::stream_protocol::acceptor m_acceptor;
    RequestHandler m_request_handler;
    std::string m_socket_path;
};
}  // namespace ipcourier::_detail

#endif  // INTER_PROCESS_COURIER_SYNCUNIXDOMAINSERVER_HPP
