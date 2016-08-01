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

namespace LiFFT {
namespace libraries {
namespace cuFFT {
namespace traits{

    /**
     * Defines the FFTW internal types for the given precision type:
     * PlanType, ComplexType
     */
    template< typename T_Precision >
    struct Types;

    template<>
    struct Types<float>
    {
        using ComplexType = cufftComplex;
        using RealType = cufftReal;
    };

    template<>
    struct Types<double>
    {
        using ComplexType = cufftDoubleComplex;
        using RealType = cufftDoubleReal;
    };

}  // namespace traits
}  // namespace cuFFT
}  // namespace libraries

namespace traits {

    template< typename T >
    struct IsBinaryCompatible< types::Real<T>, typename libraries::cuFFT::traits::Types<T>::RealType >
        :std::true_type{};

    template< typename T >
    struct IsBinaryCompatible< types::Complex<T>, typename libraries::cuFFT::traits::Types<T>::ComplexType >
        :std::true_type{};

}  // namespace traits
}  // namespace LiFFT
