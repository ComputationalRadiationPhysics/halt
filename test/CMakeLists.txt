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
 
find_package (Boost COMPONENTS unit_test_framework REQUIRED)

add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable(minIncludes minIncludes.cpp)

option(LiFFT_ENABLE_COVERAGE "Generate coverage build" OFF)
if(LiFFT_ENABLE_COVERAGE)
    if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        message(FATAL_ERROR "Coverage requires a debug build or you get false positives")
    endif()
    # Note: "--coverage" instead of "-coverage" makes this work with ccache
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage")
endif()

if(CUDA_FOUND AND LiFFT_ENABLE_CUDA)
    file(GLOB LiFFT_Test_FILES
        "*.cu"
        "*.hpp"
    )

    list(REMOVE_ITEM LiFFT_Test_FILES ${CMAKE_CURRENT_SOURCE_DIR}/testOpenCL.cpp)
    cuda_add_executable(Test ${LiFFT_Test_FILES})
    CUDA_ADD_CUFFT_TO_TARGET(Test)
else()
    file(GLOB LiFFT_Test_FILES
        "*.cpp"
        "*.hpp"
    )
    if(NOT(CLFFT_FOUND AND LiFFT_ENABLE_OPENCL))
        list(REMOVE_ITEM LiFFT_Test_FILES ${CMAKE_CURRENT_SOURCE_DIR}/testOpenCL.cpp)
    endif()
    list(REMOVE_ITEM LiFFT_Test_FILES ${CMAKE_CURRENT_SOURCE_DIR}/minIncludes.cpp)
    add_executable(Test ${LiFFT_Test_FILES})
endif()


target_link_libraries(Test ${LIBS} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_custom_command(
    TARGET Test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/writeData.py"
    ${CMAKE_BINARY_DIR}
)
add_custom_command(
    TARGET Test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/rect.tif"
    ${CMAKE_BINARY_DIR}
)
add_custom_command(
    TARGET Test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/input1.tif"
    ${CMAKE_BINARY_DIR}
)
add_custom_command(
    TARGET Test
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/input2.tif"
    ${CMAKE_BINARY_DIR}
)

