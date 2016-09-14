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

#include "libLiFFT/types/TypePair.hpp"
#include "libLiFFT/libraries/clFFT/Plan.hpp"
#include "libLiFFT/libraries/clFFT/traits/FFTType.hpp"
#include "libLiFFT/libraries/clFFT/traits/Sign.hpp"
#include "libLiFFT/policies/SafePtrCast.hpp"
#include "libLiFFT/libraries/clFFT/policies/GetInplaceMemSize.hpp"

#include <cassert>
#include <type_traits>
#include <clFFT.h>
#include <string>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

  namespace detail {
    template<
      typename T_Precision,
      bool T_isComplexIn,
      bool T_isComplexOut,
      bool T_isFwd
      >
    struct ExecutePlan
    {
      cl_int operator()(clfftPlanHandle plan,
                        cl_command_queue queue,
                        cl_mem in,
                        cl_mem out) {
        return clfftEnqueueTransform(plan,
                                     traits::Sign< T_isFwd >::value,
                                     1, &queue, // queue
                                     0, NULL, NULL, // events
                                     &in, // input buffer
                                     &out,  // output buffer
                                     NULL   // tmp buffer
          );
      }
    };
  }  // namespace detail

  template<
    typename T_Precision,
    class T_InOutTypes,
    bool T_isFwd,
    bool T_isInplace,
    unsigned T_numDims,
    bool T_isComplexIn,
    bool T_isComplexOut
    >
  struct ExecutePlan
  {
  private:
    using Precision = T_Precision;
    using Input = typename T_InOutTypes::First;
    using Output = typename T_InOutTypes::Second;
    static constexpr bool isFwd = T_isFwd;
    static constexpr unsigned numDims = T_numDims;
    static constexpr bool isInplace = T_isInplace;
    static constexpr bool isComplexIn = T_isComplexIn;
    static constexpr bool isComplexOut = T_isComplexOut;

    using Executer = detail::ExecutePlan< Precision, isComplexIn, isComplexOut, isFwd >;

  public:
    template< class T_Plan, class T_Copier >
    void
    operator()(T_Plan& plan,
               Input& input,
               Output& output,
               bool useInplaceForHost,
               const T_Copier& copy) {
      using LiFFT::policies::safe_ptr_cast;
      static_assert(!isInplace, "Cannot be used for inplace transforms!");
      size_t memsizeIn = sizeof(Precision)*input.getNumElements() * (isComplexIn?2:1);
      size_t memsizeOut = sizeof(Precision)*output.getNumElements() * (isComplexOut?2:1);
      cl_mem pIn = 0;
      if( plan.InDevicePtr )
      {
        // host w unpitched reals to pitched reals in device buffer
        if(useInplaceForHost && !isComplexIn) {
          auto fullExtents = input.getFullExtents();
          size_t w = fullExtents[numDims-1] * sizeof(Precision);
          size_t pitch = (fullExtents[numDims-1]/2+1)*2*sizeof(Precision);
          size_t h = memsizeIn/sizeof(Precision) / fullExtents[numDims-1];
          copy.copyPitched(plan.InDevicePtr.get(),
                           input.getDataPtr(),
                           plan.queue,
                           w,
                           h,
                           pitch
            );
        }else
          copy.copy(plan.InDevicePtr.get(),
                    input.getDataPtr(),
                    memsizeIn,
                    plan.queue);
        pIn = safe_ptr_cast<cl_mem>(plan.InDevicePtr.get());
      }else if(Input::IsDeviceMemory::value)
        pIn = reinterpret_cast<cl_mem>(input.getDataPtr());
      else
        throw std::runtime_error("Neither input nor plan has device pointer.");

      cl_mem pOut;
      if(plan.OutDevicePtr)
        pOut = safe_ptr_cast<cl_mem>(plan.OutDevicePtr.get());
      else if(Output::IsDeviceMemory::value)
        pOut = reinterpret_cast<cl_mem>(output.getDataPtr());
      else if(useInplaceForHost)
        pOut = safe_ptr_cast<cl_mem>(plan.InDevicePtr.get());
      else
        throw std::runtime_error("Neither output nor plan has device pointer.");

      CHECK_CL( Executer()(plan.handle, plan.queue, pIn, pOut) );

      if(plan.OutDevicePtr || !Output::IsDeviceMemory::value) {
        if(useInplaceForHost && !isComplexOut) {
          auto fullExtents = output.getFullExtents();
          size_t w = fullExtents[numDims-1] * sizeof(Precision);
          size_t pitch = (fullExtents[numDims-1]/2+1)*2*sizeof(Precision);
          size_t h = memsizeOut/sizeof(Precision) / fullExtents[numDims-1];
          copy.copyPitched(output.getDataPtr(),
                           pOut,
                           plan.queue,
                           w,
                           h,
                           pitch
            );
        }else
          copy.copy(output.getDataPtr(), pOut, memsizeOut, plan.queue);
        CHECK_CL(clFinish(plan.queue)); // finish transfers
      }
    }

    template< class T_Plan, class T_Copier >
    void
    operator()(T_Plan& plan, Input& inOut, const T_Copier& copy) {
      using LiFFT::policies::safe_ptr_cast;
      static_assert(isInplace, "Must be used for inplace transforms!");

      cl_mem pIn = 0;
      if( plan.InDevicePtr ) {
        copy.copy(plan.InDevicePtr.get(),
                  inOut.getDataPtr(),
                  inOut.getMemSize(),
                  plan.queue);
        pIn = plan.InDevicePtr.get();
      }else if(Input::IsDeviceMemory::value)
        pIn = reinterpret_cast<cl_mem>(inOut.getDataPtr());
      else
        throw std::runtime_error("Neither inOut nor plan has device pointer.");

      cl_mem pOut = pIn;

      CHECK_CL( Executer()(plan.handle, plan.queue, pIn, pOut) );

      if( plan.InDevicePtr ) {
        auto pOutHost = inOut.getDataPtr();
        copy.copy(pOutHost,
                  pOut,
                  inOut.getMemSize(),
                  plan.queue);
        CHECK_CL(clFinish(plan.queue)); // finish transfers
      }
    }
  };

}  // namespace policies
}  // namespace cuFFT
}  // namespace libraries
}  // namespace LiFFT
