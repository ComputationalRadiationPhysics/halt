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
     * Evaluates to true type if the given Array-like type is strided
     */
    template< typename T, typename T_SFINAE = void >
    struct IsStrided;

    template< typename T >
    struct IsStrided< T, void_t< decltype(T::isStrided) > >: std::integral_constant< bool, T::isStrided >{};

}  // namespace traits
}  // namespace LiFFT
