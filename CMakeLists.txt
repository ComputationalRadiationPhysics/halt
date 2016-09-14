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
# OpenCL+clFFT
###############################################################################
find_package(OpenCL)
if(OPENCL_FOUND)
  find_package(clFFT)
  set(LiFFT_ENABLE_OPENCL ${CLFFT_FOUND} CACHE BOOL "Use OpenCL/clFFT")
  if(CLFFT_FOUND AND LiFFT_ENABLE_OPENCL)
    set(LIBS ${LIBS} ${CLFFT_LIBRARIES} ${OPENCL_LIBRARIES})
    include_directories(${CLFFT_INCLUDE_DIRS})
  endif()
endif()

###############################################################################
# CUDA
###############################################################################
find_package(CUDA)
set(LiFFT_ENABLE_CUDA ${CUDA_FOUND} CACHE BOOL "Use CUDA")
if(CUDA_FOUND AND LiFFT_ENABLE_CUDA)
  if(LiFFT_ENABLE_OPENCL)
    message(FATAL_ERROR "cuFFT as well as clFFT have been found and you have enabled both of them, please use e.g. -DLiFFT_ENABLE_CUDA=0 -DLiFFT_ENABLE_OPENCL=1.")
  endif()
  list(APPEND CUDA_NVCC_FLAGS_RELEASE "-O3 -DNDEBUG")
  list(APPEND CUDA_NVCC_FLAGS_DEBUG "-O0 -g")
  # Older cmakes used the default compiler, not the CMAKE C compiler
  if(${CMAKE_VERSION} VERSION_LESS 2.8.10)
    list(APPEND CUDA_NVCC_FLAGS "-ccbin ${CMAKE_C_COMPILER}")
  endif()
  # http://stackoverflow.com/questions/34996295/trying-to-get-cuda-7-5-to-work-with-gcc-5-x
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND ${CUDA_VERSION_STRING} EQUAL "7.5")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
      list(APPEND CUDA_NVCC_FLAGS "-D_MWAITXINTRIN_H_INCLUDED")
      message(WARNING "CUDA 7.5 and gcc5+ probably will fail to compile liFFT/cufft, try gcc4.8.2.")
    endif()
  endif()
  set(CUDA_PROPAGATE_HOST_FLAGS OFF)
endif(CUDA_FOUND AND LiFFT_ENABLE_CUDA)


###############################################################################
# FFTW
###############################################################################
if((CUDA_FOUND AND LiFFT_ENABLE_CUDA)
   OR (CLFFT_FOUND AND LiFFT_ENABLE_OPENCL))
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
    if(CLFFT_FOUND AND LiFFT_ENABLE_OPENCL)
      add_definitions(-DWITH_OPENCL)
    endif()
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

