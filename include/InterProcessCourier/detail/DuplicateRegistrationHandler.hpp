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

#ifndef INTER_PROCESS_COURIER_DUPLICATE_REGISTRATION_HANDLER_FWD_HPP
#define INTER_PROCESS_COURIER_DUPLICATE_REGISTRATION_HANDLER_FWD_HPP

#include <functional>
#include <string>

#include <InterProcessCourier/SyncCommons.hpp>

namespace ipcourier::_detail {
bool registerDuplicateRequestResponsePair(DuplicateRequestResponsePairRegistrationStrategy strategy,
                                          std::function<void(const std::string&, const std::string&)> register_function,
                                          const std::string& request_name,
                                          const std::string& response_name);
}  // namespace ipcourier::_detail

#endif  // INTER_PROCESS_COURIER_DUPLICATE_REGISTRATION_HANDLER_FWD_HPP
