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

#include "liFFT/void_t.hpp"

namespace LiFFT {
namespace traits {

    /**
     * Returns the number of dimensions for the given array-like type
     */
    template< typename T, typename T_SFINAE = void >
    struct NumDims;

    template< typename T >
    struct NumDims< T, void_t< decltype(T::numDims) > >: std::integral_constant< unsigned, T::numDims >{};

    template< typename T >
    struct NumDims< T& >: NumDims<T>{};

}  // namespace traits
}  // namespace LiFFT
