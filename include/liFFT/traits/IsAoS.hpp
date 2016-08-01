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

#include "liFFT/traits/IsComplex.hpp"

namespace LiFFT {
namespace traits {

    /**
     * Evaluates to a true type if the given complex type is an Array of Structs
     * (that is the data has only 1 pointer and complex values are interleaved),
     * false otherwise (you have 2 pointers for real and imaginary data)
     * Defaults to true for real values
     */
    template< typename T, typename T_SFINAE = void >
    struct IsAoS: std::true_type
    {
        static_assert(!IsComplex<T>::value, "A user specialization must be provided for Complex types");
    };


    template< typename T >
    struct IsAoS< T, void_t< decltype(T::isAoS) > >: std::integral_constant< bool, T::isAoS >{};


}  // namespace traits
}  // namespace LiFFT
