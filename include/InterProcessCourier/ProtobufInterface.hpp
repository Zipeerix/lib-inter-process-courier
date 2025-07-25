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

/**
 * @file ProtobufInterface.hpp
 * @brief Utilities for working with Protocol Buffers in InterProcessCourier.
 */

#ifndef INTER_PROCESS_COURIER_PROTOBUF_INTERFACE_HPP
#define INTER_PROCESS_COURIER_PROTOBUF_INTERFACE_HPP

#include <google/protobuf/Message.h>
#include <google/protobuf/empty.pb.h>

namespace ipcourier {
/// Type alias for a base type of all Protocol Buffer messages
using BaseProtoType = google::protobuf::Message;

/// Type alias for an empty protobuf message
using NoMessage = google::protobuf::Empty;

/**
 * @brief Concept to ensure template types derive from the Protocol Buffer Message base class.
 * @tparam T The type to check
 */
template <typename T>
concept IsDerivedFromProtoMessage = std::derived_from<T, BaseProtoType>;
}  // namespace ipcourier

#endif  // INTER_PROCESS_COURIER_PROTOBUF_INTERFACE_HPP
