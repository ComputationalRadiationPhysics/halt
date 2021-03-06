# This file is part of libLiFFT.
#
# libLiFFT is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# libLiFFT is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with libLiFFT.  If not, see <www.gnu.org/licenses/>.

# Hack for C++11 Support in eclipse
# https://public.kitware.com/Bug/bug_relationship_graph.php?bug_id=15316&graph=relation
# The CMAKE_CXX_COMPILER_ARG1 must be set before the project() call to have cmake set up
# the correct flags that are later used by eclipse
if (${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++11" CACHE STRING "C++ version for eclipse" FORCE)
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)
endif ()

include(CheckCXXCompilerFlag)
macro(CheckAndAddFlag flag)
	set(VarName ${flag})
	string(REPLACE "+" "X" VarName ${VarName})
	CHECK_CXX_COMPILER_FLAG(${flag} FLAG_${VarName}_SUPPORTED)
	if(FLAG_${VarName}_SUPPORTED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
		list(APPEND CUDA_NVCC_FLAGS "-Xcompiler ${flag}")
	endif()
endmacro()

project(libLiFFT)
cmake_minimum_required(VERSION 2.8.0)

set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} $ENV{HOME}/Software/tiff $ENV{HOME}/Software/FFTW)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

###############################################################################
# TIFF
###############################################################################
find_package(TIFF REQUIRED)
set(LIBS ${LIBS} ${TIFF_LIBRARIES})
include_directories(${TIFF_INCLUDE_DIR})

###############################################################################
# CUDA
###############################################################################
find_package(CUDA)
set(LiFFT_ENABLE_CUDA ${CUDA_FOUND} CACHE BOOL "Use CUDA")
set(CUDA_PROPAGATE_HOST_FLAGS OFF)
list(APPEND CUDA_NVCC_FLAGS_RELEASE "-O3 -DNDEBUG")
list(APPEND CUDA_NVCC_FLAGS_DEBUG "-O0 -g")
# Older cmakes used the default compiler, not the CMAKE C compiler
if(${CMAKE_VERSION} VERSION_LESS 2.8.10)
    list(APPEND CUDA_NVCC_FLAGS "-ccbin ${CMAKE_C_COMPILER}")
endif()

###############################################################################
# FFTW
###############################################################################
if(CUDA_FOUND AND LiFFT_ENABLE_CUDA)
    find_package(FFTW)
else()
    find_package(FFTW REQUIRED)
endif()
if(FFTW_FOUND)
    set(LIBS ${LIBS} ${FFTW_LIB} ${FFTW_F_LIB})
    include_directories(${FFTW_INCLUDE_DIRS})
endif()

###############################################################################
# Boost
###############################################################################
find_package(Boost 1.48.0 COMPONENTS program_options system filesystem REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Use the CMake variable available at 3.1 and up
if(${CMAKE_VERSION} VERSION_LESS 3.1)
    add_definitions(-std=c++11)
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
# Required by CUDA w/o CUDA_PROPAGATE_HOST_FLAGS=ON
list(APPEND CUDA_NVCC_FLAGS "-std=c++11")

CheckAndAddFlag(-Wall)
CheckAndAddFlag(-Wextra)
CheckAndAddFlag(-Wshadow)
CheckAndAddFlag(/W3)

if(CUDA_FOUND AND LiFFT_ENABLE_CUDA)
    add_definitions(-DWITH_CUDA)
    cuda_add_executable(fftTiffImg fftTiffImg/main.cu)
    CUDA_ADD_CUFFT_TO_TARGET(fftTiffImg)
    cuda_add_executable(reportVolumes reportVolumes/main.cu)
    CUDA_ADD_CUFFT_TO_TARGET(reportVolumes)
else()
	add_executable(fftTiffImg fftTiffImg/main.cpp)
	add_executable(reportVolumes reportVolumes/main.cpp)
endif()

add_executable(tiff2PDF tiff2Pdf/tiff2pdf.cpp)

target_link_libraries(tiff2PDF ${TIFF_LIBRARIES})
target_link_libraries(fftTiffImg ${LIBS})
target_link_libraries(reportVolumes ${LIBS})

add_subdirectory(test)

enable_testing()
add_test(NAME Main_Test COMMAND Test)

