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

#include <format>

#include <../include/InterProcessCourier/detail/DuplicateRegistrationHandler.hpp>

namespace ipcourier::_detail {
bool registerDuplicateRequestResponsePair(
    const DuplicateRequestResponsePairRegistrationStrategy strategy,
    const std::function<void(const std::string&, const std::string&)>& register_function,
    const std::string& request_name,
    const std::string& response_name) {
    switch (strategy) {
        case DuplicateRequestResponsePairRegistrationStrategy::SilentOverride:
            register_function(request_name, response_name);
            return true;
        case DuplicateRequestResponsePairRegistrationStrategy::SilentIgnore:
            return true;
        case DuplicateRequestResponsePairRegistrationStrategy::IndicateIgnore:
            return false;
        case DuplicateRequestResponsePairRegistrationStrategy::Throw:
            throw std::invalid_argument(std::format(
                "Duplicate request/response pair registration attempted for: {}:{}", request_name, response_name));
        default:
            throw std::logic_error("Unknown DuplicateRequestResponsePairRegistrationStrategy");
    }
}
}  // namespace ipcourier::_detail
