# CMakeLists.txt
#
# (c) copyright 2017 coord.e
#
# This file is part of scopion.
#
# scopion is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# scopion is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with scopion.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8)

add_subdirectory(assembly)
add_subdirectory(ast)
add_subdirectory(parser)

add_library(scopion SHARED scopion.cpp)

target_link_libraries(scopion assembly ast parser ${Boost_LIBRARIES} ${LLVM_LIBRARIES})

INSTALL(TARGETS scopion LIBRARY DESTINATION lib)
