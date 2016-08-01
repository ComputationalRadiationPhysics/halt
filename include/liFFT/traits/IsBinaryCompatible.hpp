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

#include "liFFT/c++14_types.hpp"

namespace LiFFT {
namespace traits {

    /**
     * Implementation of \ref IsBinaryCompatible
     * Specialize this!
     */
    template< typename T_Src, typename T_Dest >
    struct IsBinaryCompatibleImpl: std::false_type{};

    template< typename T >
    struct IsBinaryCompatibleImpl< T, T >: std::true_type{};

    /**
     * Returns true if this types are binary compatible,
     * that is a conversion between pointers to those types is "safe"
     */
    template< typename T_Src, typename T_Dest >
    struct IsBinaryCompatible
            : std::integral_constant<
              bool,
              IsBinaryCompatibleImpl< std::remove_cv_t<T_Src>, std::remove_cv_t<T_Dest> >::value ||
              IsBinaryCompatibleImpl< std::remove_cv_t<T_Dest>, std::remove_cv_t<T_Src> >::value
              >{};

}  // namespace traits
}  // namespace LiFFT
