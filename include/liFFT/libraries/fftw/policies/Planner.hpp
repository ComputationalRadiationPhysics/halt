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

#include "liFFT/policies/SafePtrCast.hpp"
#include "liFFT/types/TypePair.hpp"
#include "liFFT/libraries/fftw/traits/Sign.hpp"
#include "liFFT/libraries/fftw/policies/CreatePlan.hpp"
#include "liFFT/c++14_types.hpp"
#include <cassert>

namespace LiFFT {
namespace libraries {
namespace fftw {
namespace policies {

    template<
            typename T_Precision,
            class T_InOutTypes,
            bool T_isFwd,
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
    template< typename T_Precision, class T_InOutTypes, bool T_isFwd, bool T_isInplace, unsigned T_numDims, bool T_isComplexIn, bool T_isComplexOut >
    struct Planner< T_Precision, T_InOutTypes, T_isFwd, T_isInplace, T_numDims, T_isComplexIn, T_isComplexOut, true, true, false, false >
    {
    private:
        using Precision = T_Precision;
        static constexpr unsigned numDims = T_numDims;
        using Input  = typename T_InOutTypes::First;
        using Output = typename T_InOutTypes::Second;
        static constexpr bool isFwd        = T_isFwd;
        static constexpr bool isInplace    = T_isInplace;
        static constexpr bool isComplexIn  = T_isComplexIn;
        static constexpr bool isComplexOut = T_isComplexOut;
        using ComplexType = typename traits::LibTypes< Precision >::ComplexType;
        using LibInType = std::conditional_t<
                                    isComplexIn,
                                    ComplexType,
                                    Precision
                                >*;
        using LibOutType = std::conditional_t<
                                    isComplexOut,
                                    ComplexType,
                                    Precision
                                >*;
        static_assert(isComplexIn || isComplexOut, "Real2Real transform not supported");
        static_assert(isComplexIn || isFwd, "Real2Complex is always a forward transform");
        static_assert(isComplexOut || !isFwd, "Complex2Real is always a backward transform");

    public:
        using PlanType = typename traits::LibTypes<T_Precision>::PlanType;

        PlanType
        operator()(Input& input, Output& output, const unsigned flags = FFTW_ESTIMATE)
        {
            using LiFFT::policies::safe_ptr_cast;
            static_assert(!isInplace, "Cannot be used for inplace transforms!");
            auto extents(input.getExtents());
            auto extentsOut(output.getExtents());
            for(unsigned i=0; i<numDims; ++i){
                unsigned eIn = extents[i];
                unsigned eOut = extentsOut[i];
                // Same extents in all dimensions unless we have a C2R or R2C and compare the last dimension
                bool dimOk = (eIn == eOut || (i+1 == numDims && !(isComplexIn && isComplexOut)));
                // Half input size for first dimension of R2C
                dimOk &= (isComplexIn || i+1 != numDims || eIn/2+1 == eOut);
                // Half output size for first dimension of C2R
                dimOk &= (isComplexOut || i+1 != numDims || eIn == eOut/2+1);
                if(!dimOk)
                    throw std::runtime_error("Dimension " + std::to_string(i) + ": Extents mismatch");
            }
            return policies::CreatePlan<Precision>().Create(
                    numDims,
                    reinterpret_cast<const int*>(input.getFullExtents().data()),
                    safe_ptr_cast<LibInType>(input.getDataPtr()),
                    safe_ptr_cast<LibOutType>(output.getDataPtr()),
                    traits::Sign<isFwd>::value,
                    flags
                    );
        }

        PlanType
        operator()(Input& inOut, const unsigned flags = FFTW_ESTIMATE)
        {
            using LiFFT::policies::safe_ptr_cast;
            static_assert(isInplace, "Must be used for inplace transforms!");
            return policies::CreatePlan<Precision>().Create(
                    numDims,
                    reinterpret_cast<const int*>(inOut.getFullExtents().data()),
                    safe_ptr_cast<LibInType>(inOut.getDataPtr()),
                    reinterpret_cast<LibOutType>(safe_ptr_cast<LibInType>(inOut.getDataPtr())),
                    traits::Sign<isFwd>::value,
                    flags
                    );
        }
    };

}  // namespace policies
}  // namespace fftw
}  // namespace libraries
}  // namespace LiFFT
