/***************************************************************************
 * InterProcessCourier Copyright (C) 2025  Ziperix                        *
 *                                                                        *
 * This program is free software: you can redistribute it and/or modify   *
 * it under the terms of the GNU General Public License as published by   *
 * the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                    *
 *                                                                        *
 * This program is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 * GNU General Public License for more details.                           *
 *                                                                        *
 * You should have received a copy of the GNU General Public License      *
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * @file SyncCommons.hpp
 * @brief Common definitions and utilities for a synchronous client and server in InterProcessCourier.
 */

#ifndef INTER_PROCESS_COURIER_SYNCCOMMONS_HPP
#define INTER_PROCESS_COURIER_SYNCCOMMONS_HPP

namespace ipcourier {
/**
 * @brief Defines strategies for handling duplicate request/response handler registrations.
 *
 * This enumeration determines how the `SyncServer` behaves when a handler is registered
 * for a request type that already has an existing handler registered.
 * @see SyncServerOptions::duplicate_registration_strategy
 */
enum class DuplicateRequestResponsePairRegistrationStrategy {
    SilentOverride,  ///< New registration overrides the old one. Always return true.
    SilentIgnore,    ///< New registration is ignored if a pair was already registered. Always return true.
    IndicateIgnore,  ///< Same as SilentIgnore but return a boolean indicating success.
    Throw            ///< Throw std::invalid_argument if a pair was already registered, otherwise return true.
};
}  // namespace ipcourier

#endif  // INTER_PROCESS_COURIER_SYNCCOMMONS_HPP
