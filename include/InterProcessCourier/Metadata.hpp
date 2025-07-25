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
 * @file Metadata.hpp
 * @brief Utilities for versioning and other library metadata.
 */

#ifndef INTER_PROCESS_COURIER_METADATA_HPP
#define INTER_PROCESS_COURIER_METADATA_HPP

#include <string>

namespace ipcourier {
/**
 * @brief Enum class representing the communication protocol used by InterProcessCourier.
 *
 * This enum defines the protocols that can be used for communication between processes.
 * Currently, only Google Protocol Buffer (Protobuf) is supported.
 */
enum class CommunicationProtocol {
    Protobuf  ///< Google Protocol Buffers
};

/**
 * @brief Gets the protocol that InterProcessCourier library uses for communication.
 *
 * Returns the current protocol of the InterProcessCourier library at compile time.
 * Controlled using conan option 'protocol' or using INTER_PROCESS_COURIER_PROTOCOL macro.
 * If the protocol is not recognized, google protobuf will be used by default.
 *
 * @return CommunicationProtocol enum value indicating the protocol used.
 */
CommunicationProtocol getProtocol();

/**
 * @brief Gets the version string of the InterProcessCourier library
 *
 * Returns the current version of the InterProcessCourier library at compile time.
 * The version follows semantic versioning (MAJOR.MINOR.PATCH) format.
 *
 * @return The library version string (e.g., "1.2.3")
 */
std::string getLibraryVersion();
}  // namespace ipcourier

#endif  // INTER_PROCESS_COURIER_METADATA_HPP
