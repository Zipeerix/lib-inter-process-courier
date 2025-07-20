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

/**
 * @file SyncServer.hpp
 * @brief Defines the synchronous server interface for InterProcessCourier,
 * enabling communication with clients using Protocol Buffer messages over Unix Domain Sockets.
 */

#ifndef INTER_PROCESS_COURIER_SERVER_HPP
#define INTER_PROCESS_COURIER_SERVER_HPP

#include <InterProcessCourier/Error.hpp>
#include <InterProcessCourier/ProtobufTools.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>
#include <expected>
#include <InterProcessCourier/ThirdPartyFwd.hpp>

namespace ipcourier {
/**
 * @brief Enumeration of specific error codes for the synchronous server.
 */
enum class SyncServerError {
    UnknownError,               ///< An unspecified error occurred.
    HandlerNotRegistered,       ///< No handler is registered for the received Protocol Buffer message type.
    RuntimeError,               ///< An error occurred during the execution of a message handler.
    UnableToDeserializeMessage, ///< The server failed to deserialize an incoming message into a Protocol Buffer.
    UnableToSerializeMessage,   ///< The server failed to serialize a response Protocol Buffer message.
};

/**
 * @brief Type alias for the result of synchronous server operations.
 *
 * This alias represents an `std::expected` type where success is indicated by `SuccessType`
 * and failure by an `Error` object containing a `SyncServerError`.
 *
 * @tparam SuccessType The type returned on successful operation.
 */
template <typename SuccessType>
using SyncServerResult = std::expected<SuccessType, Error<SyncServerError> >;

class SyncUnixDomainServer;

/**
 * @brief A synchronous server for inter-process communication using Protocol Buffers
 * over Unix Domain Sockets.
 *
 * This class provides a high-level interface for accepting client connections,
 * receiving Protocol Buffer requests, dispatching them to registered handlers,
 * and sending Protocol Buffer responses.
 */
class SyncServer {
    using GenericHandler = std::function<std::string(const BaseProtoType&)>;

public:
    /**
     * @brief Type alias for a specific handler function for a given request and response type.
     *
     * This alias defines the signature for a handler function that takes a `RequestType`
     * Protocol Buffer message and returns a `ResponseType` Protocol Buffer message.
     *
     * @tparam RequestType The type of the Protocol Buffer request message.
     * @tparam ResponseType The type of the Protocol Buffer response message.
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    using HandlerForSpecificType = std::function<ResponseType(const RequestType&)>;

    /**
     * @brief Constructs a SyncServer instance.
     *
     * @param io_context Boost::Asio io_context, note that this is a synchronous server, so no async operation will run
     * @param socket_addr The path to the Unix Domain Socket file to bind to and listen on.
     */
    SyncServer(boost::asio::io_context& io_context, const std::string& socket_addr);

    ~SyncServer();

    /**
     * @brief Registers a handler function for a specific Protocol Buffer request type.
     *
     * When a client sends a request of `RequestType`, the provided `handler` function
     * will be invoked with the deserialized request message. The return value of the
     * handler (a `ResponseType` message) will be serialized and sent back to the client.
     * If a `RequestType` was previously registered to a different ` ResponseType `, then it will be overwritten.
     *
     * @tparam RequestType The type of the Protocol Buffer request message this handler processes.
     * Must derive from `google::protobuf::Message`.
     * @tparam ResponseType The type of the Protocol Buffer response message this handler returns.
     * Must derive from `google::protobuf::Message`.
     * @param handler The function to be called when a `RequestType` message is received.
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    void registerHandler(HandlerForSpecificType<RequestType, ResponseType> handler) {
        const auto type_name = RequestType::descriptor()->full_name();
        m_handlers[type_name] = [handler = std::move(handler)](const BaseProtoType& msg) {
            const auto response = handler(static_cast<const RequestType&>(msg));
            return makePayloadFromProto(response);
        };
    }

    /**
     * @brief Starts the server, binding to the socket address and listening for incoming connections.
     *
     * This method enters a blocking loop, accepting client connections, receiving messages,
     * dispatching them to registered handlers, and sending responses.
     *
     * @return SyncServerResult<void> A result indicating success or an error if the server
     * fails to start or encounters a critical runtime error.
     */
    SyncServerResult<void> start();

private:
    std::unordered_map<std::string, GenericHandler> m_handlers;
    std::unique_ptr<SyncUnixDomainServer> m_server;

    SyncServerResult<SerializedProtoPayload> acceptMessage(const SerializedProtoPayload& serialized);

    std::unique_ptr<BaseProtoType> tryParseProtobuf(const SerializedProtoPayload& serialized) const;
};
} // namespace ipcourier

#endif  // INTER_PROCESS_COURIER_SERVER_HPP
