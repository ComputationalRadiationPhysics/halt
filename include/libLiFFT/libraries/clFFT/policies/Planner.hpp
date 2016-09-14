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
#include "libLiFFT/libraries/clFFT/traits/Types.hpp"
#include "libLiFFT/libraries/clFFT/policies/GetInplaceMemSize.hpp"

#include <clFFT.h>
#include <cassert>
#include <limits>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

  /**
   * Functor that creates a plan.
   * Takes references to the plan, input, output and their allocators
   */
  template<
    typename T_Context,
    typename T_Precision,
    class T_InOutTypes,
    bool T_isInplace,
    unsigned T_numDims,
    bool T_isComplexIn,
    bool T_isComplexOut,
    bool T_isAoSIn,
    bool T_isAoSOut,
    bool T_isStridedIn,
    bool T_isStridedOut
    >
  struct Planner;

  /**
   * Both AoS, Both non-strided
   */
  template<
    typename T_Context,
    typename T_Precision,
    class T_InOutTypes,
    bool T_isInplace,
    unsigned T_numDims,
    bool T_isComplexIn,
    bool T_isComplexOut
    >
  struct Planner< T_Context, T_Precision, T_InOutTypes, T_isInplace, T_numDims, T_isComplexIn, T_isComplexOut, true, true, false, false >
  {
  private:
    using Precision = T_Precision;
    static constexpr unsigned numDims = T_numDims;
    using Input  = typename T_InOutTypes::First;
    using Output = typename T_InOutTypes::Second;
    static constexpr bool isInplace    = T_isInplace;
    static constexpr bool isComplexIn  = T_isComplexIn;
    static constexpr bool isComplexOut = T_isComplexOut;

    static_assert(isComplexIn || isComplexOut, "Real2Real conversion not supported");
    //static_assert(T_isComplexIn || T_isFwd, "Real2Complex is always a forward transform");
    //static_assert(T_isComplexOut || !T_isFwd, "Complex2Real is always a backward transform");
    static_assert(numDims > 0 && numDims <= 3, "Only 1D-3D is supported");


  private:
    template< class T_Plan, class T_Extents >
    void
    createPlan(T_Plan& plan, T_Extents& fullExtents, bool useInplaceForHost=false) {
      assert(plan.handle==0);
      size_t cldims[numDims];
      // switch order since clfft library exposes [Nz][Ny][Nx],
      //  while liFFT uses [Nx][Ny][Nz]
      //  (both row-major)
      // if not C2C then dims are overwritten by the strides later
      for(auto j=numDims; j>0; --j)
        cldims[numDims-j] = fullExtents[j-1];
      CHECK_CL( clfftCreateDefaultPlan(&plan.handle,
                                       plan.ctx,
                                       traits::Dims< T_numDims >::value,
                                       cldims) );
      CHECK_CL( clfftSetPlanPrecision(plan.handle,
                                      traits::FFTPrecision< T_Precision >::value) );
      CHECK_CL( clfftSetLayout(plan.handle,
                               traits::FFTLayout<T_isComplexIn,T_isComplexOut>::value,
                               traits::FFTLayout<T_isComplexOut,T_isComplexIn>::value) );
      CHECK_CL( clfftSetResultLocation(plan.handle,
                                       traits::FFTInplace<T_isInplace>::value) );

      size_t n = LiFFT::policies::getNumElementsFromExtents(fullExtents);
      size_t strides[3] = {1};
      size_t transform_strides[3] = {1};
      size_t dist = 0;
      size_t transform_dist = 0;
      if(!isComplexIn || !isComplexOut) {
        if(isInplace || useInplaceForHost) { // inplace R2C or C2R
          size_t n_complex = n / fullExtents[numDims-1] * (fullExtents[numDims-1]/2 + 1);
          strides[1] = 2*(fullExtents[numDims-1]/2+1); // reals w padding, row
          strides[2] = 2 * n_complex / fullExtents[0];
          transform_strides[1] = fullExtents[numDims-1]/2+1;
          transform_strides[2] = n_complex / fullExtents[0];
          dist = 2 * n_complex;
          transform_dist = n_complex;
        }else{ // outplace R2C or C2R
          size_t n_complex = n / fullExtents[numDims-1] * (fullExtents[numDims-1]/2 + 1);
          strides[1] = fullExtents[numDims-1];
          strides[2] = n / fullExtents[0];
          transform_strides[1] = fullExtents[numDims-1]/2+1;
          transform_strides[2] = n_complex / fullExtents[0];
          dist = n;
          transform_dist = n_complex;
        }
      }
      if(!isComplexIn) // R2C
      {
        CHECK_CL(clfftSetPlanInStride(plan.handle,
                                      traits::Dims< T_numDims >::value,
                                      strides));
        CHECK_CL(clfftSetPlanOutStride(plan.handle,
                                       traits::Dims< T_numDims >::value,
                                       transform_strides));
        CHECK_CL(clfftSetPlanDistance(plan.handle, dist, transform_dist));
      }else if(!isComplexOut){ // C2R
        CHECK_CL(clfftSetPlanOutStride(plan.handle,
                                       traits::Dims< T_numDims >::value,
                                       strides));
        CHECK_CL(clfftSetPlanInStride(plan.handle,
                                      traits::Dims< T_numDims >::value,
                                      transform_strides));
        CHECK_CL(clfftSetPlanDistance(plan.handle, transform_dist, dist));
      }
      CHECK_CL(clfftBakePlan(plan.handle,
                             1, // number of queues
                             &plan.queue,
                             nullptr, // callback
                             nullptr)); // user data
    }

    void checkSize(size_t size) {
      if(size > std::numeric_limits<unsigned>::max())
        throw std::runtime_error("Size is too big (clFFT limitation) :" + std::to_string(size));
    }
  public:

    template< class T_Plan, class T_Allocator >
    void
    operator()(T_Plan& plan, Input& input, Output& output, bool useInplaceForHost, const T_Allocator& alloc) {
      static_assert(!isInplace, "Cannot be used for inplace transforms!");
      auto extents(input.getExtents());
      auto extentsOut(output.getExtents());
      for(unsigned i=0; i<numDims; ++i){
        unsigned eIn = extents[i];
        unsigned eOut = extentsOut[i];
        // Same extents in all dimensions unless we have a C2R or R2C and compare the last dimension
        bool dimOk = (eIn == eOut || (i == numDims-1 && !(isComplexIn && isComplexOut)));
        // Half input size for first dimension of R2C
        dimOk &= (isComplexIn || i != numDims-1 || eIn/2+1 == eOut);
        // Half output size for first dimension of C2R
        dimOk &= (isComplexOut || i != numDims-1 || eIn == eOut/2+1);
        if(!dimOk)
          throw std::runtime_error("Dimension " + std::to_string(i) + ": Extents mismatch");
      }
      // Need 2 counts as they are different for C2R/R2C (maybe 1 element off)
      size_t numElementsIn = input.getNumElements();
      size_t numElementsOut = output.getNumElements();

      if(useInplaceForHost && !Input::IsDeviceMemory::value && !Output::IsDeviceMemory::value){
        size_t size = std::max(numElementsIn*(isComplexIn?2:1),
                               numElementsOut*(isComplexOut?2:1)) * sizeof(Precision);
        checkSize(size);
        plan.InDevicePtr.reset( alloc.malloc(size, plan.ctx) );
      }else{
        size_t inSize = numElementsIn * sizeof(Precision) * (isComplexIn?2:1);
        size_t outSize = numElementsOut * sizeof(Precision) * (isComplexOut?2:1);
        checkSize(inSize);
        checkSize(outSize);
        if(!Input::IsDeviceMemory::value)
          plan.InDevicePtr.reset(
            alloc.malloc(inSize, plan.ctx));
        if(!Output::IsDeviceMemory::value)
          plan.OutDevicePtr.reset(
            alloc.malloc(outSize, plan.ctx));
      }
      // Always use fullExtents, that is the extents of the real container for R2C/C2R
      // For C2C it does not matter
      createPlan(plan, isComplexIn ? extentsOut : extents, useInplaceForHost);
    }

    template< class T_Plan, class T_Allocator >
    void
    operator()(T_Plan& plan, Input& inOut, const T_Allocator& alloc) {
      size_t size = policies::GetInplaceMemSize<Precision, isComplexIn, isComplexOut, numDims>::get(inOut.getFullExtents());
      checkSize(size);
      if(!Input::IsDeviceMemory::value)
        plan.InDevicePtr.reset(
          alloc.malloc(size, plan.ctx));
      createPlan(plan, inOut.getFullExtents());
    }
  };

}  // namespace policies
}  // namespace clFFT
}  // namespace libraries
}  // namespace libLiFFT
