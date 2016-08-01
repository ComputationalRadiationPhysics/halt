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

#include "liFFT/libraries/cuFFT/traits/Types.hpp"
#include "liFFT/c++14_types.hpp"

namespace LiFFT {
namespace libraries {
namespace cuFFT {
namespace traits{

    /**
     * Defines the LibIn/OutType for a configuration
     */
    template<
            typename T_Precision,
            bool T_isComplexIn,
            bool T_isComplexOut
    >
    struct LibTypes
    {
        using Precision = T_Precision;
        static constexpr bool isComplexIn = T_isComplexIn;
        static constexpr bool isComplexOut = T_isComplexOut;

        using RealType = typename Types< Precision >::RealType;
        using ComplexType = typename Types< Precision >::ComplexType;
        using InType = std::conditional_t<
                              isComplexIn,
                              ComplexType,
                              RealType
                          >;
        using OutType = std::conditional_t<
                              isComplexOut,
                              ComplexType,
                              RealType
                          >;
    };

}  // namespace traits
}  // namespace cuFFT
}  // namespace libraries
}  // namespace LiFFT
