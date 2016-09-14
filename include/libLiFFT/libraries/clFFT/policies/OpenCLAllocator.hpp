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

#include "libLiFFT/libraries/clFFT/clfft_helper.hpp"

#include <clFFT.h>
#include <CL/cl.h>
#include <iostream>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

    /**
     * Allocator
     */
    struct OpenCLAllocator
    {
        cl_mem
        malloc(size_t memSize, cl_context ctx) const
        {
          cl_int err=0;
          cl_mem mem = clCreateBuffer( ctx,
                                       CL_MEM_READ_WRITE,
                                       memSize,
                                       NULL,
                                       &err );
          CHECK_CL(err);
          return mem;
        }

        void
        free(cl_mem mem) const
        {
          CHECK_CL(clReleaseMemObject(mem));
        }
    };

}  // namespace policies
}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
