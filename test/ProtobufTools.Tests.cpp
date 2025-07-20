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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <format>
#include <InterProcessCourier/ProtobufTools.hpp>
#include "ProtoForTests.pb.h"

TEST(ProtobufTools, createProtoPayload_FormatsStringCorrectly) {
    const std::string type_name = "my.package.MyMessage";
    const std::string serialized_data = "\x08\x01\x10\x02"; // Example binary data

    const auto payload = ipcourier::createProtoPayload(type_name, serialized_data);
    ASSERT_EQ(payload, "my.package.MyMessage:\x08\x01\x10\x02");
}

TEST(ProtobufTools, createProtoPayload_HandlesEmptyTypeName) {
    const std::string type_name;
    const std::string serialized_data = "some_data";

    const auto payload = ipcourier::createProtoPayload(type_name, serialized_data);
    ASSERT_EQ(payload, ":some_data");
}

TEST(ProtobufTools, createProtoPayload_HandlesEmptySerializedData) {
    const std::string type_name = "my.package.MyMessage";
    const std::string serialized_data;

    const auto payload = ipcourier::createProtoPayload(type_name, serialized_data);
    ASSERT_EQ(payload, "my.package.MyMessage:");
}

TEST(ProtobufTools, makePayloadFromProto_SerializesHelloWorldMessage) {
    ipcourier::test_proto::HelloWorld msg;
    msg.set_message("Hello, Protobuf!");
    msg.set_integer(123);

    const auto payload = ipcourier::makePayloadFromProto(msg);
    const std::string expected_prefix = "ipcourier.test_proto.HelloWorld:";
    ASSERT_TRUE(payload.rfind(expected_prefix, 0) == 0) << "Payload does not start with expected prefix.";
    const auto serialized_data_part = payload.substr(expected_prefix.length());
    ipcourier::test_proto::HelloWorld deserialized_msg;
    ASSERT_TRUE(deserialized_msg.ParseFromString(serialized_data_part));
    ASSERT_EQ(deserialized_msg.message(), "Hello, Protobuf!");
    ASSERT_EQ(deserialized_msg.integer(), 123);
}

TEST(ProtobufTools, makePayloadFromProto_SerializesEmptyHelloWorldMessage) {
    const ipcourier::test_proto::HelloWorld msg;
    ASSERT_TRUE(msg.message().empty());
    ASSERT_EQ(msg.integer(), 0);

    const auto payload = ipcourier::makePayloadFromProto(msg);

    const std::string expected_prefix = "ipcourier.test_proto.HelloWorld:";
    ASSERT_TRUE(payload.rfind(expected_prefix, 0) == 0) << "Payload does not start with expected prefix.";

    const auto serialized_data_part = payload.substr(expected_prefix.length());
    ipcourier::test_proto::HelloWorld deserialized_msg;
    ASSERT_TRUE(deserialized_msg.ParseFromString(serialized_data_part));
    ASSERT_TRUE(deserialized_msg.message().empty());
    ASSERT_EQ(deserialized_msg.integer(), 0);
}

TEST(ProtobufTools, makeProtoFromPayload_DeserializesHelloWorldMessage) {
    ipcourier::test_proto::HelloWorld original_msg;
    original_msg.set_message("Test Message");
    original_msg.set_integer(42);
    std::string serialized_data;
    original_msg.SerializeToString(&serialized_data);
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        serialized_data
        );

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(payload);

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->message(), "Test Message");
    ASSERT_EQ(result->integer(), 42);
}

TEST(ProtobufTools, makeProtoFromPayload_DeserializesEmptyHelloWorldMessage) {
    const ipcourier::test_proto::HelloWorld original_msg; // Empty message
    std::string serialized_data;
    original_msg.SerializeToString(&serialized_data);
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        serialized_data
        );

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(payload);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->message().empty());
    ASSERT_EQ(result->integer(), 0);
}

TEST(ProtobufTools, makeProtoFromPayload_ReturnsInvalidFormatError_WhenNoDelimiter) {
    const std::string payload = "ipcourier.test_proto.HelloWorld_NoDelimiter"; // Missing colon

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::InvalidFormat);
    ASSERT_EQ(result.error().message, "Received message: ipcourier.test_proto.HelloWorld_NoDelimiter");
}

TEST(ProtobufTools, makeProtoFromPayload_ReturnsInvalidFormatError_WhenLongPayloadTruncatedInErrorMsg) {
    std::string long_payload = "ipcourier.test_proto.HelloWorld";
    long_payload += std::string(200, 'A'); // Make it longer than 128 chars

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(long_payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::InvalidFormat);
    ASSERT_TRUE(result.error().message.rfind("Received message: ipcourier.test_proto.HelloWorldA", 0) == 0);
    ASSERT_TRUE(result.error().message.find("...") != std::string::npos);
}


TEST(ProtobufTools, makeProtoFromPayload_ReturnsTypeMismatchError_WhenExpectedTypeDiffers) {
    const google::protobuf::Empty empty_msg;
    std::string serialized_data;
    empty_msg.SerializeToString(&serialized_data);
    const std::string payload = ipcourier::createProtoPayload(
        google::protobuf::Empty::descriptor()->full_name(),
        serialized_data
        );

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::TypeMismatch);
    ASSERT_EQ(result.error().message,
              std::format("Received {} but expected {}",
                  google::protobuf::Empty::descriptor()->full_name(),
                  ipcourier::test_proto::HelloWorld::descriptor()->full_name()));
}

TEST(ProtobufTools, makeProtoFromPayload_ReturnsDeserializationFailedError_WhenCorruptBinaryData) {
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        "corrupt_binary_data_that_is_not_protobuf"
        );

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::DeserializationFailed);
}

TEST(ProtobufTools, makeProtoFromPayload_ReturnsDeserializationFailedError_WhenEmptyBinaryDataForNonEmptyMessage) {
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        ""
        );

    const auto result = ipcourier::makeProtoFromPayload<ipcourier::test_proto::HelloWorld>(payload);

    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->message().empty());
    ASSERT_EQ(result->integer(), 0);
}

TEST(ProtobufTools, makeBaseProtoFromPayload_DeserializesHelloWorldMessageDynamically) {
    ipcourier::test_proto::HelloWorld original_msg;
    original_msg.set_message("Dynamic Message");
    original_msg.set_integer(99);

    std::string serialized_data;
    original_msg.SerializeToString(&serialized_data);
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        serialized_data
        );

    const auto result = ipcourier::makeBaseProtoFromPayload(payload);

    ASSERT_TRUE(result.has_value());
    ASSERT_NE(result->get(), nullptr);

    const auto* deserialized_msg = dynamic_cast<const ipcourier::test_proto::HelloWorld*>(result->get());
    ASSERT_NE(deserialized_msg, nullptr);
    ASSERT_EQ(deserialized_msg->message(), "Dynamic Message");
    ASSERT_EQ(deserialized_msg->integer(), 99);
}

TEST(ProtobufTools, makeBaseProtoFromPayload_DeserializesEmptyHelloWorldMessageDynamically) {
    const ipcourier::test_proto::HelloWorld original_msg;
    std::string serialized_data;
    original_msg.SerializeToString(&serialized_data);
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        serialized_data
        );

    const auto result = ipcourier::makeBaseProtoFromPayload(payload);

    ASSERT_TRUE(result.has_value());
    ASSERT_NE(result->get(), nullptr);

    const auto* deserialized_msg = dynamic_cast<const ipcourier::test_proto::HelloWorld*>(result->get());
    ASSERT_NE(deserialized_msg, nullptr);
    ASSERT_TRUE(deserialized_msg->message().empty());
    ASSERT_EQ(deserialized_msg->integer(), 0);
}


TEST(ProtobufTools, makeBaseProtoFromPayload_ReturnsInvalidFormatError_WhenNoDelimiter) {
    const std::string payload = "ipcourier.test_proto.HelloWorld_NoDelimiter_Base"; // Missing colon

    const auto result = ipcourier::makeBaseProtoFromPayload(payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::InvalidFormat);
    ASSERT_EQ(result.error().message, "Received message: ipcourier.test_proto.HelloWorld_NoDelimiter_Base");
}

TEST(ProtobufTools, makeBaseProtoFromPayload_ReturnsDeserializationFailedError_WhenUnknownMessageType) {
    const std::string payload = ipcourier::createProtoPayload(
        "non.existent.MessageType",
        "some_data"
        );

    const auto result = ipcourier::makeBaseProtoFromPayload(payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::DeserializationFailed);
    ASSERT_EQ(result.error().message, "Description for non.existent.MessageType not found");
}

TEST(ProtobufTools, makeBaseProtoFromPayload_ReturnsDeserializationFailedError_WhenCorruptBinaryData) {
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        "this_is_not_valid_protobuf_binary_data"
        );

    const auto result = ipcourier::makeBaseProtoFromPayload(payload);

    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(result.error().type, ipcourier::ProtoPayloadParseError::DeserializationFailed);
    ASSERT_TRUE(
        result.error().message.rfind("Unable to deserialize as ipcourier.test_proto.HelloWorld. Message:", 0) == 0);
}

TEST(ProtobufTools,
     makeBaseProtoFromPayload_ReturnsDeserializationFailedError_WhenEmptyBinaryDataForNonEmptyMessage) {
    const std::string payload = ipcourier::createProtoPayload(
        ipcourier::test_proto::HelloWorld::descriptor()->full_name(),
        ""
        );

    const auto result = ipcourier::makeBaseProtoFromPayload(payload);

    ASSERT_TRUE(result.has_value());
    ASSERT_NE(result->get(), nullptr);

    const auto* deserialized_msg = dynamic_cast<const ipcourier::test_proto::HelloWorld*>(result->get());
    ASSERT_NE(deserialized_msg, nullptr);
    ASSERT_TRUE(deserialized_msg->message().empty());
    ASSERT_EQ(deserialized_msg->integer(), 0);
}
