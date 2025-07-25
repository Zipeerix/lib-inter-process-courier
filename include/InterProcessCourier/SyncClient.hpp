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

#include <expected>
#include <memory>
#include <string>

#include <InterProcessCourier/Error.hpp>
#include <InterProcessCourier/SyncCommons.hpp>
#include <InterProcessCourier/detail/DetailFwd.hpp>
#include <InterProcessCourier/detail/ProtobufTools.hpp>
#include <InterProcessCourier/detail/ThirdPartyFwd.hpp>

namespace ipcourier {
/**
 * @brief Enumeration of specific error codes for the synchronous client.
 */
enum class SyncClientError {
    UnknownError,                ///< An unspecified error occurred.
    BadRequestToResponsePair,    ///< The requested Protocol Buffer type pair (request/response) is not registered
    UnableToReflectMappings,     ///< The client failed to reflect the request-response mappings from the server.
    UnableToConnectToServer,     ///< The client failed to establish a connection with the server.
    UnableToSendMessage,         ///< The client failed to send a message to the server.
    UnableToReceiveMessage,      ///< The client failed to receive a message from the server.
    UnableToParseReturnedProto,  ///< The client received a message but failed to parse it into a Protocol Buffer.
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

/**
 * @brief Defines strategies for validating the consistency of request and response Protocol Buffer message pairs.
 *
 * This enumeration helps ensure that the `SyncClient` sends and receives messages that conform
 * to the expected types and structures defined by the server's API.
 * @see SyncClientOptions::validate_req_res_pair_strategy
 */
enum class ValidateRequestResponsePairStrategy {
    NoValidation,        ///< No validation is performed on outgoing requests
    ManualRegistration,  ///< Request and response message type pairs are manually registered for validation.
    ServerReflection,    ///< The client queries the server to get pairs for validation.
};

/**
 * @brief Structure to hold various configuration options for the SyncClient.
 * @see SyncClient
 */
struct SyncClientOptions {
    /**
     * @brief Specifies the strategy for validating request-response pairs during communication.
     * This ensures that the client's outgoing requests and incoming responses adhere to
     * the expected Protobuf message types and structures defined by the server.
     * @see ValidateRequestResponsePairStrategy
     */
    ValidateRequestResponsePairStrategy validate_req_res_pair_strategy =
        ValidateRequestResponsePairStrategy::ServerReflection;

    /**
     * @brief Strategy for handling duplicate request/response pair registrations.
     *
     * This option determines what the registration functions returns when a handler is registered
     * for a request type that already has a registered handler.
     *
     * Has no impact if SyncClientOptions::validate_req_res_pair_strategy is set to
     * ValidateRequestResponsePairStrategy::ServerReflection
     *
     * @see DuplicateRequestResponsePairRegistrationStrategy
     */
    DuplicateRequestResponsePairRegistrationStrategy duplicate_registration_strategy =
        DuplicateRequestResponsePairRegistrationStrategy::IndicateIgnore;
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
     * @param socket_addr The path to the Unix Domain Socket file on which the server listens.
     * @param client_options Various settings relating to the client. @see SyncClientOptions
     */
    SyncClient(std::string socket_addr, SyncClientOptions client_options);

    ~SyncClient();

    /**
     * @brief Attempts to establish a connection with the server at the specified socket address.
     *
     * @return SyncClientResult<void> A result indicating success or an error if the connection fails.
     * @retval SyncClientError::UnableToReflectMappings If the client failed to reflect the request-response mappings
     * from the server.
     */
    SyncClientResult<void> connect();

    /**
     * @brief Registers the expected response Protocol Buffer type for a given request Protocol Buffer type.
     *
     * This method is used to explicitly define the valid request-response pairs. When client-side validation
     * is enabled via `SyncClientOptions::validate_req_res_pair`, calling `sendRequest` with a
     * `RequestType` that has an unexpected `ResponseType` (i.e., not registered here) will result in an error,
     * preventing the request from being sent. This helps ensure type safety and correctness in inter-process
     * communication.
     *
     * \warning What this function returns depends on the `SyncClientOptions::duplicate_registration_strategy` setting.
     *
     * @tparam RequestType The Protocol Buffer message type that represents the request.
     * Must derive from `google::protobuf::Message`.
     * @tparam ResponseType The Protocol Buffer message type that represents the expected response for `RequestType`.
     * Must derive from `google::protobuf::Message`.
     *  @returns Boolean value, what it indicated depends on the `SyncClientOptions::duplicate_registration_strategy`
     * setting.
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    bool registerRequestResponsePair() {
        const auto request_name = RequestType::descriptor()->full_name();
        const auto response_name = ResponseType::descriptor()->full_name();

        if (m_request_response_pairs.contains(request_name) &&
            m_client_options.validate_req_res_pair_strategy != ValidateRequestResponsePairStrategy::ServerReflection) {
            return registerDuplicateRequestResponsePair(request_name, response_name);
        }

        registerValidatedRequestResponsePair(request_name, response_name);
        return true;
    }

    /**
     * @brief Sends a Protocol Buffer request and receives a Protocol Buffer response synchronously.
     *
     * This templated method serializes the `RequestType` message, sends it to the server,
     * waits for a response, and then deserializes the response into a `ResponseType` message.
     *
     * @tparam RequestType The type of the Protocol Buffer request message (must derive from google::protobuf::Message).
     * @tparam ResponseType The expected type of the Protocol Buffer response message (must derive from
     * google::protobuf::Message).
     * @param request The Protocol Buffer message to send as a request.
     * @return SyncClientResult<ResponseType> A result containing the deserialized response message on success,
     * or an error if sending, receiving, or parsing fails.
     * @retval ResponseType The deserialized Protocol Buffer response message.
     * @retval SyncClientError::BadRequestToResponsePair If the `RequestType`, `ResponseType` pair is not registered
     * when the validation setting is enabled.
     * @retval SyncClientError::UnableToSendMessage If the request could not be sent.
     * @retval SyncClientError::UnableToReceiveMessage If no response was received or an error occurred during
     * reception.
     * @retval SyncClientError::UnableToParseReturnedProto If the received payload could not be parsed into
     * `ResponseType`.
     */
    template <IsDerivedFromProtoMessage RequestType, IsDerivedFromProtoMessage ResponseType>
    SyncClientResult<ResponseType> sendRequest(const RequestType& request) {
        if (m_client_options.validate_req_res_pair_strategy != ValidateRequestResponsePairStrategy::NoValidation) {
            const auto request_name = RequestType::descriptor()->full_name();
            const auto response_name = ResponseType::descriptor()->full_name();

            auto it = m_request_response_pairs.find(request_name);

            const auto found = it != m_request_response_pairs.end();
            if (!found || it->second != response_name) {
                const auto expected_response_name = found ? it->second : "<Not Registered>";
                const auto strategy = m_client_options.validate_req_res_pair_strategy;

                return std::unexpected(Error(
                    SyncClientError::BadRequestToResponsePair,
                    std::format(
                        "Request type '{}' expects response type '{}', but '{}' was provided. Current strategy: {}",
                        request_name,
                        expected_response_name,
                        response_name,
                        strategy)));
            }
        }

        const auto serialized_request = _detail::makePayloadFromProto(request);
        const auto send_and_receive_result = sendAndReceiveMessage(serialized_request);
        if (!send_and_receive_result.has_value()) {
            return std::unexpected(send_and_receive_result.error());
        }

        const auto response = send_and_receive_result.value();
        const auto proto_parse_result = _detail::makeProtoFromPayload<ResponseType>(response);
        if (!proto_parse_result.has_value()) {
            return std::unexpected(
                Error(SyncClientError::UnableToParseReturnedProto, proto_parse_result.error().message));
        }

        return proto_parse_result.value();
    }

private:
    SyncClientOptions m_client_options;
    std::string m_socket_addr;
    std::unique_ptr<boost::asio::io_context> m_io_context;

    std::unique_ptr<_detail::SyncUnixDomainClient> m_client;

    std::unordered_map<std::string, std::string> m_request_response_pairs;

    bool registerDuplicateRequestResponsePair(const std::string& request_name, const std::string& response_name);

    void registerValidatedRequestResponsePair(const std::string& request_name, const std::string& response_name);

    SyncClientResult<_detail::SerializedProtoPayload> sendAndReceiveMessage(
        const _detail::SerializedProtoPayload& serialized) const;

    SyncClientResult<void> reflectRequestResponseMappingPairs();
};
}  // namespace ipcourier

template <>
struct std::formatter<ipcourier::ValidateRequestResponsePairStrategy> {
public:
    static constexpr auto parse(const std::format_parse_context& ctx) {
        return ctx.begin();
    }

    static auto format(const ipcourier::ValidateRequestResponsePairStrategy strategy, std::format_context& ctx) {
        return std::format_to(ctx.out(), "{}", convertStrategyToString(strategy));
    }

private:
    static constexpr std::string_view convertStrategyToString(
        const ipcourier::ValidateRequestResponsePairStrategy strategy) {
        switch (strategy) {
            case ipcourier::ValidateRequestResponsePairStrategy::NoValidation:
                return "NoValidation";
            case ipcourier::ValidateRequestResponsePairStrategy::ManualRegistration:
                return "ManualRegistration";
            case ipcourier::ValidateRequestResponsePairStrategy::ServerReflection:
                return "ServerReflection";
            default:
                return "Unknown";
        }
    }
};

#endif  // INTER_PROCESS_COURIER_CLIENT_HPP
