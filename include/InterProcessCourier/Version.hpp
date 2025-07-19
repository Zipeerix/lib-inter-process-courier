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

#ifndef INTER_PROCESS_COURIER_VERSION_HPP
#define INTER_PROCESS_COURIER_VERSION_HPP

#include <string>

namespace ipcourier {
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

#endif  // INTER_PROCESS_COURIER_VERSION_HPP
