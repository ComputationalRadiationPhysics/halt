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
     * Copier that uses standard cudaMemCpy
     */
    struct CudaMemCpy
    {
    private:
        void cpy1D(void* ptrTo, void* ptrFrom, size_t size, cudaMemcpyKind direction) const
        {
            CudaSafeCall(cudaMemcpy(ptrTo, ptrFrom, size, direction));
        }

        void cpyPitched(void* ptrTo, void* ptrFrom, size_t w, size_t h, size_t pitchTo, size_t pitchFrom, cudaMemcpyKind direction) const
        {
            if( pitchTo == pitchFrom && (h == 1 || pitchTo == w) )
                cpy1D(ptrTo, ptrFrom, pitchTo*h, direction);
            CudaSafeCall(cudaMemcpy2D(ptrTo, pitchTo, ptrFrom, pitchFrom, w, h, direction));
        }
    public:
        template< typename T >
        void H2D(T* ptrTo, T* ptrFrom, size_t w, size_t h, size_t pitchTo, size_t pitchFrom) const
        {
            cpyPitched(ptrTo, ptrFrom, w, h, pitchTo, pitchFrom, cudaMemcpyHostToDevice);
        }

        template< typename T >
        void D2H(T* ptrTo, T* ptrFrom, size_t w, size_t h, size_t pitchTo, size_t pitchFrom) const
        {
            cpyPitched(ptrTo, ptrFrom, w, h, pitchTo, pitchFrom, cudaMemcpyDeviceToHost);
        }
    };

}  // namespace policies
}  // namespace cuFFT
}  // namespace libraries
}  // namespace LiFFT
