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

#include "liFFT/traits/IsBinaryCompatible.hpp"
#include "liFFT/types/Complex.hpp"

namespace LiFFT {
namespace libraries {
namespace fftw {
namespace traits{

    /**
     * Defines the FFTW internal types for the given precision type:
     * PlanType, ComplexType
     */
    template< typename T_Precision >
    struct LibTypes;

    template<>
    struct LibTypes<float>
    {
        using PlanType = fftwf_plan;
        using ComplexType = fftwf_complex;
    };

    template<>
    struct LibTypes<double>
    {
        using PlanType = fftw_plan;
        using ComplexType = fftw_complex;
    };

}  // namespace traits
}  // namespace fftw
}  // namespace libraries

namespace policies {

    template< typename T >
    struct SafePtrCast_Impl< typename libraries::fftw::traits::LibTypes<T>::ComplexType*, T* >
        :Ptr2Ptr< typename libraries::fftw::traits::LibTypes<T>::ComplexType*, T* >{};

}  // namespace policies

namespace traits {

    template< typename T >
    struct IsBinaryCompatible< types::Complex<T>, typename libraries::fftw::traits::LibTypes<T>::ComplexType >
        :std::true_type{};

}  // namespace traits
}  // namespace LiFFT
