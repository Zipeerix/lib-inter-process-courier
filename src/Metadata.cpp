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

#include <InterProcessCourier/Metadata.hpp>

namespace ipcourier {
CommunicationProtocol getProtocol() {
    const static auto protocol = std::string(INTER_PROCESS_COURIER_PROTOCOL);
    if (protocol == "protobuf") {
        return CommunicationProtocol::Protobuf;
    }

    throw CommunicationProtocol::Protobuf;
}

std::string getLibraryVersion() {
    return INTER_PROCESS_COURIER_LIB_VERSION;
}
}  // namespace ipcourier
