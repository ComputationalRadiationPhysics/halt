/* This file is part of libLiFFT.
 *
 * libLiFFT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libLiFFT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libLiFFT.  If not, see <www.gnu.org/licenses/>.
 */
 
#pragma once

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cufft.h>
#include <sstream>

#ifndef CUDA_DISABLE_ERROR_CHECKING
#define CHECK_ERROR(ans) ::LiFFT::libraries::cuFFT::check_cuda((ans), "", #ans, __FILE__, __LINE__)
#define CHECK_LAST(msg) ::LiFFT::libraries::cuFFT::check_cuda(cudaGetLastError(), msg, "CHECK_LAST", __FILE__, __LINE__)
#else
#define CHECK_ERROR(ans) {}
#define CHECK_LAST(msg) {}
#endif

namespace LiFFT {
namespace libraries {
namespace cuFFT {

    inline
    void throw_error(int code,
                     const char* error_string,
                     const char* msg,
                     const char* func,
                     const char* file,
                     int line) {
        throw std::runtime_error("CUDA error "
                                 +std::string(msg)
                                 +" "+std::string(error_string)
                                 +" ["+std::to_string(code)+"]"
                                 +" "+std::string(file)
                                 +":"+std::to_string(line)
                                 +" "+std::string(func)
            );
    }

    static const char* cufftResultToString(cufftResult error) {
        switch (error) {
        case CUFFT_SUCCESS:
            return "CUFFT_SUCCESS";

        case CUFFT_INVALID_PLAN:
            return "CUFFT_INVALID_PLAN";

        case CUFFT_ALLOC_FAILED:
            return "CUFFT_ALLOC_FAILED";

        case CUFFT_INVALID_TYPE:
            return "CUFFT_INVALID_TYPE";

        case CUFFT_INVALID_VALUE:
            return "CUFFT_INVALID_VALUE";

        case CUFFT_INTERNAL_ERROR:
            return "CUFFT_INTERNAL_ERROR";

        case CUFFT_EXEC_FAILED:
            return "CUFFT_EXEC_FAILED";

        case CUFFT_SETUP_FAILED:
            return "CUFFT_SETUP_FAILED";

        case CUFFT_INVALID_SIZE:
            return "CUFFT_INVALID_SIZE";

        case CUFFT_UNALIGNED_DATA:
            return "CUFFT_UNALIGNED_DATA";

        case CUFFT_INVALID_DEVICE:
            return "CUFFT_INVALID_DEVICE";

        case CUFFT_PARSE_ERROR:
            return "CUFFT_PARSE_ERROR";

        case CUFFT_NO_WORKSPACE:
            return "CUFFT_NO_WORKSPACE";

        case CUFFT_NOT_IMPLEMENTED:
            return "CUFFT_NOT_IMPLEMENTED";

        case CUFFT_LICENSE_ERROR:
            return "CUFFT_LICENSE_ERROR";

        case CUFFT_INCOMPLETE_PARAMETER_LIST:
            return "CUFFT_INCOMPLETE_PARAMETER_LIST";

        case CUFFT_NOT_SUPPORTED:
          return "CUFFT_NOT_SUPPORTED";
        }
        return "<unknown>";
    }

    inline
    void check_cuda(cudaError_t code, const char* msg, const char *func, const char *file, int line) {
        if (code != cudaSuccess) {
            throw_error(static_cast<int>(code),
                        cudaGetErrorString(code), msg, func, file, line);
        }
    }
    inline
    void check_cuda(cufftResult code, const char* /*msg*/,  const char *func, const char *file, int line) {
        if (code != CUFFT_SUCCESS) {
            throw_error(static_cast<int>(code),
                        cufftResultToString(code), "cufft", func, file, line);
        }
    }
} // CuFFT
} // libraries
} // LiFFT
