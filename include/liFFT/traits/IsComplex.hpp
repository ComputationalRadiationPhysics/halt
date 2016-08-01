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
     * Evaluates to true type if the given type is Complex, false for Real
     */
    template< typename T, typename T_SFINAE = void >
    struct IsComplex;

    /**
     * Specialization for pointers
     */
    template< typename T >
    struct IsComplex< T* >: IsComplex< T >{};

    template< typename T >
    struct IsComplex< T& >: IsComplex< T >{};

    template<>
    struct IsComplex< double >: std::false_type{};

    template<>
    struct IsComplex< float >: std::false_type{};

    template< typename T >
    struct IsComplex< T, void_t< decltype(T::isComplex) > >: std::integral_constant< bool, T::isComplex >{};

}  // namespace traits
}  // namespace LiFFT
