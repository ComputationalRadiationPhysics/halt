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
#include "liFFT/libraries/cuFFT/cudaSafeCall.hpp"

namespace LiFFT {
namespace libraries {
namespace cuFFT {
namespace policies {

    /**
     * Allocator that uses cudaMalloc/cudaFree
     */
    struct CudaAllocator
    {
        template< typename T >
        T*
        malloc(size_t memSize) const
        {
            void* ptr;
            CudaSafeCall(cudaMalloc(&ptr, memSize));
            return reinterpret_cast<T*>(ptr);
        }

        template< typename T >
        void
        free(T* ptr) const
        {
            CudaSafeCall(cudaFree(ptr));
        }
    };

}  // namespace policies
}  // namespace cuFFT
}  // namespace libraries
}  // namespace LiFFT
