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

#include <type_traits>
#include "libLiFFT/traits/RemoveModifiers.hpp"
#include "libLiFFT/void_t.hpp"
#include "libLiFFT/c++14_types.hpp"

namespace LiFFT {
namespace traits {

    /**
     * This checks if a type T has a member `type`.
     *
     * liFFTs data wrapper want to know if T is integral type or not to derive
     * whether the data lives in complex or in real space.
     * A non-integral type must be liFFT compatible, i.e., it must have
     * a ::type and a ::isComplex member.
     * By the integration of OpenCL as a possible backend there also exist
     * a wrapper for such things on the OpenCL level.
     * The data type `cl_mem` is non-integral, but must be treated
     * like an integral one by liFFT.
     */
    template<typename T, typename = void>
    struct has_type_member : std::false_type { };
    template<typename T>
    struct has_type_member<T, void_t< typename T::type >> : std::true_type { };

    /**
     * Specialize this to return the integral type of a given memory representation
     */
    template< typename T, typename T_SFINAE = void >
    struct IntegralTypeImpl;

    template< typename T >
    struct IntegralTypeImpl<
        T,
        std::enable_if_t<std::is_integral<T>::value || std::is_floating_point<T>::value || has_type_member<T>::value==false>
        >
    {
        using type = T;
    };


    // Forward declaration
    template< typename T >
    struct IntegralType;

    /**
     * Specialization for structs with a type-member
     * Hint: More Specializations on differently named type aliases are possible with the void_t
     */
    template< typename T >
    struct IntegralTypeImpl< T, void_t< typename T::type > >: IntegralType< typename T::type >{};

    /**
     * Returns the integral type of a given memory representation
     */
    template< typename T >
    struct IntegralType{
        // Removes all modifiers and calls IntegralTypeImpl
        using type = typename IntegralTypeImpl<
                typename RemoveModifer<T>::type
              >::type;
    };

    template< typename T >
    using IntegralType_t = typename IntegralType<T>::type;

}  // namespace traits
}  // namespace LiFFT
