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
 * @file SyncClient.hpp
 * @brief Defines the synchronous client interface for InterProcessCourier,
 * enabling communication with a server using Protocol Buffer messages over Unix Domain Sockets.
 */

#ifndef INTER_PROCESS_COURIER_CLIENT_HPP
#define INTER_PROCESS_COURIER_CLIENT_HPP

#include <InterProcessCourier/Error.hpp>
#include <InterProcessCourier/ProtobufTools.hpp>
#include <InterProcessCourier/ThirdPartyFwd.hpp>
#include <memory>
#include <string>
#include <expected>

namespace ipcourier {
/**
 * @brief Enumeration of specific error codes for the synchronous client.
 */
enum class SyncClientError {
    UnknownError,               ///< An unspecified error occurred.
    BadRequestToResponsePair,   ///< The requested Protocol Buffer type pair (request/response) is not registered
    UnableToConnectToServer,    ///< The client failed to establish a connection with the server.
    UnableToSendMessage,        ///< The client failed to send a message to the server.
    UnableToReceiveMessage,     ///< The client failed to receive a message from the server.
    UnableToParseReturnedProto, ///< The client received a message but failed to parse it into a Protocol Buffer.
};

/**
 * @brief Type alias for the result of synchronous client operations.
 *
 * This alias represents an `std::expected` type where success is indicated by `SuccessType`
 * and failure by an `Error` object containing a `SyncClientError`.
 *
 * @tparam SuccessType The type returned on successful operation.
 */
template <typename SuccessType>
using SyncClientResult = std::expected<SuccessType, Error<SyncClientError> >;

class SyncUnixDomainClient;

/**
 * @brief Structure to hold various configuration options for the SyncClient.
 */
struct SyncClientOptions {
    /**
     * @brief The path to the Unix Domain Socket file the client should connect to.
     */
    std::string socket_addr;

    /**
     * @brief A boolean flag indicating whether the client should validate the request-response pair.
     *
     * If set to `true`, the client will perform checks to ensure the response
     * received corresponds correctly to the request sent in sendRequest.
     * If `false`, such validations will be skipped.
     */
    bool validate_req_res_pair = true;
};

/**
 * @brief A synchronous client for inter-process communication using Protocol Buffers
 * over Unix Domain Sockets.
 *
 * This class provides a high-level interface for sending Protocol Buffer requests
 * and receiving Protocol Buffer responses from a server. It abstracts away the
 * underlying socket communication details.
 */
class SyncClient {
public:
    /**
     * @brief Constructs a SyncClient instance.
     *
     * @param io_context Boost::Asio io_context, note that this is a synchronous client, so no async operation will run
     * @param client_options Various settings relating to the client. @see SyncClientOptions
     */
    SyncClient(boost::asio::io_context& io_context, SyncClientOptions client_options);

    ~SyncClient();

    /**
     * @brief Attempts to establish a connection with the server at the specified socket address.
     *
     * @return SyncClientResult<void> A result indicating success or an error if connection fails.
     */
    SyncClientResult<void> connect() const;

    /**
     * @brief Registers the expected response Protocol Buffer type for a given request Protocol Buffer type.
     *
     * This method is used to explicitly define the valid request-response pairs. When client-side validation
     * is enabled via `SyncClientOptions::validate_req_res_pair`, calling `sendRequest` with a
     * `RequestType` that has an unexpected `ResponseType` (i.e., not registered here) will result in an error,
     * preventing the request from being sent. This helps ensure type safety and correctness in inter-process communication.
     *
     * @tparam RequestType The Protocol Buffer message type that represents the request.
     * Must derive from `google::protobuf::Message`.
     * @tparam ResponseType The Protocol Buffer message type that represents the expected response for `RequestType`.
     * Must derive from `google::protobuf::Message`.
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    void registerRequestResponsePair() {
        const auto request_name = RequestType::descriptor()->full_name();
        const auto response_name = ResponseType::descriptor()->full_name();

        m_request_response_pairs[request_name] = response_name;
    }

    /**
     * @brief Sends a Protocol Buffer request and receives a Protocol Buffer response synchronously.
     *
     * This templated method serializes the `RequestType` message, sends it to the server,
     * waits for a response, and then deserializes the response into a `ResponseType` message.
     *
     * @tparam RequestType The type of the Protocol Buffer request message (must derive from google::protobuf::Message).
     * @tparam ResponseType The expected type of the Protocol Buffer response message (must derive from google::protobuf::Message).
     * @param request The Protocol Buffer message to send as a request.
     * @return SyncClientResult<ResponseType> A result containing the deserialized response message on success,
     * or an error if sending, receiving, or parsing fails.
     * @retval ResponseType The deserialized Protocol Buffer response message.
     * @retval SyncClientError::BadRequestToResponsePair If the `RequestType`, `ResponseType` pair is not registered when the validation setting is enabled.
     * @retval SyncClientError::UnableToSendMessage If the request could not be sent.
     * @retval SyncClientError::UnableToReceiveMessage If no response was received or an error occurred during reception.
     * @retval SyncClientError::UnableToParseReturnedProto If the received payload could not be parsed into `ResponseType`.
     * @see makePayloadFromProto
     * @see makeProtoFromPayload
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    SyncClientResult<ResponseType> sendRequest(const RequestType& request) {
        if (m_client_options.validate_req_res_pair) {
            const auto request_name = RequestType::descriptor()->full_name();
            const auto response_name = ResponseType::descriptor()->full_name();

            auto it = m_request_response_pairs.find(request_name);
            if (it == m_request_response_pairs.end() || it->second != response_name) {
                return std::unexpected(Error(SyncClientError::BadRequestToResponsePair,
                                             std::format(
                                                 "Request type '{}' expects response type '{}', but '{}' was provided or not registered.",
                                                 request_name,
                                                 (it != m_request_response_pairs.end()
                                                      ? it->second
                                                      : "<Not Registered>"),
                                                 response_name)));
            }
        }

        const auto serialized_request = makePayloadFromProto(request);
        const auto send_and_receive_result = sendAndReceiveMessage(serialized_request);
        if (!send_and_receive_result.has_value()) {
            return std::unexpected(send_and_receive_result.error());
        }
        const auto response = send_and_receive_result.value();
        const auto proto_parse_result = makeProtoFromPayload<ResponseType>(response);
        if (!proto_parse_result.has_value()) {
            return std::unexpected(Error(SyncClientError::UnableToParseReturnedProto,
                                         proto_parse_result.error().message));
        }

        return proto_parse_result.value();
    }

private:
    SyncClientOptions m_client_options;
    std::unique_ptr<SyncUnixDomainClient> m_client;

    std::unordered_map<std::string, std::string> m_request_response_pairs;

    SyncClientResult<std::string> sendAndReceiveMessage(const SerializedProtoPayload& serialized) const;
};
} // namespace ipcourier

#endif  // INTER_PROCESS_COURIER_CLIENT_HPP
