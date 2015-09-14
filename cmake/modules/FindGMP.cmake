#
# Minetest
# Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# https://github.com/minetest/minetest/blob/master/cmake/Modules/FindGMP.cmake

option(ENABLE_SYSTEM_GMP "Use GMP from system" TRUE)
mark_as_advanced(GMP_LIBRARY GMP_INCLUDE_DIR)
set(USE_SYSTEM_GMP FALSE)

if(ENABLE_SYSTEM_GMP)
	find_library(GMP_LIBRARY NAMES libgmp.so)
	find_path(GMP_INCLUDE_DIR NAMES gmp.h)

	if(GMP_LIBRARY AND GMP_INCLUDE_DIR)
		message (STATUS "Using GMP provided by system.")
		set(USE_SYSTEM_GMP TRUE)
	else()
		message (STATUS "Detecting GMP from system failed.")
	endif()
else()
	message (STATUS "Detecting GMP from system disabled! (ENABLE_SYSTEM_GMP=0)")
endif()

if(NOT USE_SYSTEM_GMP)
	message(STATUS "Using bundled mini-gmp library.")
	set(GMP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/gmp)
	set(GMP_LIBRARY gmp)
	add_subdirectory(gmp)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG GMP_LIBRARY GMP_INCLUDE_DIR)
