#[[
###########################################################################
#  InterProcessCourier Copyright (C) 2025  Ziperix                        #
#                                                                         #
#  This program is free software: you can redistribute it and/or modify   #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation, either version 3 of the License, or      #
#  (at your option) any later version.                                    #
#                                                                         #
#  This program is distributed in the hope that it will be useful,        #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of         #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          #
#  GNU General Public License for more details.                           #
#                                                                         #
#  You should have received a copy of the GNU General Public License      #
#  along with this program.  If not, see <https://www.gnu.org/licenses/>. #
###########################################################################
]]

cmake_minimum_required(VERSION 3.20)

project(InterProcessCourier_Example_Basic LANGUAGES CXX)

find_package(lib-inter-process-courier REQUIRED)

add_executable(InterProcessCourier_Example_Basic src/main.cpp)
target_link_libraries(InterProcessCourier_Example_Basic
                      PRIVATE lib-inter-process-courier::lib-inter-process-courier)
