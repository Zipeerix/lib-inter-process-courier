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

#include <expected>
#include <format>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <InterProcessCourier/Error.hpp>
#include <InterProcessCourier/SyncCommons.hpp>
#include <InterProcessCourier/detail/DetailFwd.hpp>
#include <InterProcessCourier/detail/DuplicateRegistrationHandler.hpp>
#include <InterProcessCourier/detail/ProtobufTools.hpp>
#include <InterProcessCourier/detail/ThirdPartyFwd.hpp>

namespace ipcourier {
/**
 * @brief Enumeration of specific error codes for the synchronous server.
 */
enum class SyncServerError {
    UnknownError,                ///< An unspecified error occurred.
    HandlerNotRegistered,        ///< No handler is registered for the received Protocol Buffer message type.
    RuntimeError,                ///< An error occurred during the execution of a message handler.
    UnableToDeserializeMessage,  ///< The server failed to deserialize an incoming message into a Protocol Buffer.
    UnableToSerializeMessage,    ///< The server failed to serialize a response Protocol Buffer message.
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

/**
 * @brief Structure to hold various configuration options for the SyncServer.
 * @see SyncServer
 */
struct SyncServerOptions {
    /**
     * @brief Strategy for handling duplicate request/response pair registrations.
     *
     * This option determines what the registration functions returns when a handler is registered
     * for a request type that already has a registered handler.
     *
     * @see DuplicateRequestResponsePairRegistrationStrategy
     */
    DuplicateRequestResponsePairRegistrationStrategy duplicate_registration_strategy =
        DuplicateRequestResponsePairRegistrationStrategy::IndicateIgnore;
};

/**
 * @brief A synchronous server for inter-process communication using Protocol Buffers
 * over Unix Domain Sockets.
 *
 * This class provides a high-level interface for accepting client connections,
 * receiving Protocol Buffer requests, dispatching them to registered handlers,
 * and sending Protocol Buffer responses.
 */
class SyncServer {
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
     * @param socket_addr The path to the Unix Domain Socket file to bind to and listen on.
     * @param server_options Various settings relating to the server. @see SyncServerOptions
     */
    SyncServer(std::string socket_addr, SyncServerOptions server_options);

    ~SyncServer();

    /**
     * @brief Registers a handler function for a specific Protocol Buffer request type.
     *
     * When a client sends a request of `RequestType`, the provided `handler` function
     * will be invoked with the deserialized request message. The return value of the
     * handler (a `ResponseType` message) will be serialized and sent back to the client.
     * Handlers should be registered before calling `start()` as the client might use reflection to discover
     * request/response pairs on connect.
     *
     * \warning What this function returns depends on the `SyncServerOptions::duplicate_registration_strategy` setting.
     *
     * @tparam RequestType The type of the Protocol Buffer request message this handler processes.
     * Must derive from `google::protobuf::Message`.
     * @tparam ResponseType The type of the Protocol Buffer response message this handler returns.
     * Must derive from `google::protobuf::Message`.
     * @param handler The function to be called when a `RequestType` message is received.
     * @returns Boolean value, what it indicated depends on the `SyncServerOptions::duplicate_registration_strategy`
     * setting.
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    bool registerHandler(HandlerForSpecificType<RequestType, ResponseType> handler) {
        const auto request_name = RequestType::descriptor()->full_name();
        const auto response_name = ResponseType::descriptor()->full_name();
        if (m_handlers.contains(request_name)) {
            return registerDuplicateRequestResponsePair(request_name, response_name, handler);
        }

        registerValidatedRequestResponsePair<RequestType, ResponseType>(request_name, response_name, handler);
        return true;
    }

    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    bool registerDuplicateRequestResponsePair(const std::string& request_name,
                                              const std::string& response_name,
                                              HandlerForSpecificType<RequestType, ResponseType> handler) {
        const auto register_handler = [this, handler](const auto& handler_request_name,
                                                      const auto& handler_response_name) {
            this->registerValidatedRequestResponsePair(handler_request_name, handler_response_name, handler);
        };
        return _detail::registerDuplicateRequestResponsePair(
            m_server_options.duplicate_registration_strategy, register_handler, request_name, response_name);
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
    SyncServerResult<void> start() const;

private:
    using GenericHandler = std::function<std::string(const BaseProtoType&)>;

    SyncServerOptions m_server_options;
    std::string m_socket_addr;
    std::unique_ptr<boost::asio::io_context> m_io_context;

    std::unordered_map<std::string, GenericHandler> m_handlers;
    std::unordered_map<std::string, std::string> m_request_response_pairs;
    std::unique_ptr<_detail::SyncUnixDomainServer> m_server;

    SyncServerResult<_detail::SerializedProtoPayload> acceptMessage(const _detail::SerializedProtoPayload& serialized);

    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    void registerValidatedRequestResponsePair(const std::string& request_name,
                                              const std::string& response_name,
                                              HandlerForSpecificType<RequestType, ResponseType> handler) {
        m_handlers[request_name] = [handler = std::move(handler)](const BaseProtoType& msg) {
            const auto response = handler(static_cast<const RequestType&>(msg));
            return _detail::makePayloadFromProto(response);
        };
        m_request_response_pairs[request_name] = response_name;
    }
};
}  // namespace ipcourier

template <>
struct std::formatter<ipcourier::SyncServerError> {
public:
    static constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    static auto format(const ipcourier::SyncServerError error, std::format_context& ctx) {
        return std::format_to(ctx.out(), "{}", convertSyncServerErrorToString(error));
    }

private:
    static constexpr std::string_view convertSyncServerErrorToString(const ipcourier::SyncServerError error_type) {
        switch (error_type) {
            case ipcourier::SyncServerError::UnknownError:
                return "Unknown error";
            case ipcourier::SyncServerError::HandlerNotRegistered:
                return "Handler not registered";
            case ipcourier::SyncServerError::RuntimeError:
                return "Runtime error";
            case ipcourier::SyncServerError::UnableToDeserializeMessage:
                return "Unable to deserialize message";
            case ipcourier::SyncServerError::UnableToSerializeMessage:
                return "Unable to serialize message";

            default:
                return "<Unknown>";
        }
    }
};

#endif  // INTER_PROCESS_COURIER_SERVER_HPP
