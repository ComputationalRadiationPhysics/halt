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

#include "liFFT/libraries/fftw/fftw3Include.h"
#include "liFFT/libraries/fftw/traits/LibTypes.hpp"
#include "liFFT/ignore_unused.hpp"

namespace LiFFT {
namespace libraries {
namespace fftw {
namespace policies{

    /**
     * Creates a plan for the given precision type
     */
    template< typename T_Precision >
    struct CreatePlan;

    template<>
    struct CreatePlan<float>
    {
        using PlanType = typename traits::LibTypes<float>::PlanType;
        using ComplexType = typename traits::LibTypes<float>::ComplexType;
        using RealType = float;

        static_assert(!std::is_same<RealType, ComplexType>::value, "Need different types for Real/Complex");

        PlanType
        Create(int rank, const int* n, ComplexType* in, ComplexType* out, int sign, unsigned flags)
        {
            return fftwf_plan_dft(rank, n, in, out, sign, flags);
        }

        PlanType
        Create(int rank, const int* n, RealType* in, ComplexType* out, int sign, unsigned flags)
        {
            assert(sign == FFTW_FORWARD);
            ignore_unused(sign);
            return fftwf_plan_dft_r2c(rank, n, in, out, flags);
        }

        PlanType
        Create(int rank, const int* n, ComplexType* in, RealType* out, int sign, unsigned flags)
        {
            assert(sign == FFTW_BACKWARD);
            ignore_unused(sign);
            return fftwf_plan_dft_c2r(rank, n, in, out, flags);
        }
    };

    template<>
    struct CreatePlan<double>
    {
        using PlanType = typename traits::LibTypes<double>::PlanType;
        using ComplexType = typename traits::LibTypes<double>::ComplexType;
        using RealType = double;

        static_assert(!std::is_same<RealType, ComplexType>::value, "Need different types for Real/Complex");

        PlanType
        Create(int rank, const int* n, ComplexType* in, ComplexType* out, int sign, unsigned flags)
        {
            return fftw_plan_dft(rank, n, in, out, sign, flags);
        }

        PlanType
        Create(int rank, const int* n, RealType* in, ComplexType* out, int sign, unsigned flags)
        {
            assert(sign == FFTW_FORWARD);
            ignore_unused(sign);
            return fftw_plan_dft_r2c(rank, n, in, out, flags);
        }

        PlanType
        Create(int rank, const int* n, ComplexType* in, RealType* out, int sign, unsigned flags)
        {
            assert(sign == FFTW_BACKWARD);
            ignore_unused(sign);
            return fftw_plan_dft_c2r(rank, n, in, out, flags);
        }
    };

}  // namespace policies
}  // namespace fftw
}  // namespace libraries
}  // namespace LiFFT
