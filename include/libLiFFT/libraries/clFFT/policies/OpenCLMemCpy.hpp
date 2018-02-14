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
#include <CL/cl.h>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

    template<bool T_Async=false>
    struct Blocking {
        constexpr static cl_bool value = CL_TRUE;
    };

    template<>
    struct Blocking<true> {
        constexpr static cl_bool value = CL_FALSE;
    };

    /**
     * Copier
     * https://github.com/clMathLibraries/clFFT
     */
    template<bool T_Async=false>
    struct OpenCLMemCpy
    {

      /* H2D */
      void copyPitched(cl_mem dst,
                       const void* src,
                       cl_command_queue queue,
                       size_t w,
                       size_t h,
                       size_t pitch) const
      {
        const size_t offset[3] = {0, 0, 0};
        size_t region[3] = {w, h, 1};
        CHECK_CL(clEnqueueWriteBufferRect( queue,
                                           dst,
                                           Blocking<T_Async>::value,
                                           offset, // buffer origin
                                           offset, // host origin
                                           region,
                                           pitch, // buffer row pitch
                                           0, // buffer slice pitch
                                           0, // host row pitch
                                           0, // host slice pitch
                                           src,
                                           0, // num_events_in_wait_list
                                           nullptr, // event_wait_list
                                           nullptr )); // event

      }
      /* D2H */
      void copyPitched(void* dst,
                       cl_mem src,
                       cl_command_queue queue,
                       size_t w,
                       size_t h,
                       size_t pitch) const
      {
        const size_t offset[3] = {0, 0, 0};
        size_t region[3] = {w, h, 1};
        CHECK_CL(clEnqueueReadBufferRect( queue,
                                          src,
                                          Blocking<T_Async>::value,
                                          offset, // buffer origin
                                          offset, // host origin
                                          region,
                                          pitch, // buffer row pitch
                                          0, // buffer slice pitch
                                          0, // host row pitch
                                          0, // host slice pitch
                                          dst,
                                          0, // num_events_in_wait_list
                                          nullptr, // event_wait_list
                                          nullptr )); // event
      }

      /* H2D */
      void copy(cl_mem dst, const void* src, size_t size, cl_command_queue queue) const
      {
        CHECK_CL( clEnqueueWriteBuffer(queue, dst,
                                       Blocking<T_Async>::value,
                                       0, size, src, 0, NULL, NULL) );
      }
      /* D2H */
      void copy(void* dst, cl_mem src, size_t size, cl_command_queue queue) const
      {
        CHECK_CL( clEnqueueReadBuffer(queue, src,
                                      Blocking<T_Async>::value,
                                      0, size, dst, 0, NULL, NULL) );
      }
      /* D2D */
      void copy(cl_mem dst, cl_mem src, size_t size, cl_command_queue queue) const
      {
        CHECK_CL( clEnqueueCopyBuffer(queue, src, dst, 0, 0, size, 0, NULL, NULL) );
      }

    };

}  // namespace policies
}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
