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

#include "liFFT/accessors/ArrayAccessor.hpp"

namespace LiFFT {
namespace traits {

    /**
     * Returns the default accessor for the given type
     */
    template< typename T, typename T_SFINAE = void >
    struct IdentityAccessor{
        using type = typename T::IdentityAccessor;
    };

    /**
     * Specialization for pointer types
     */
    template< typename T >
    struct IdentityAccessor<T*>{
        using type = accessors::ArrayAccessor<false>;
    };

    /**
     * Specialization for reference types
     */
    template< typename T >
    struct IdentityAccessor<T&>: IdentityAccessor<T>{};

    /**
     * Specialization for const types
     */
    template< typename T >
    struct IdentityAccessor<const T>: IdentityAccessor<T>{};

    template< typename T >
    using IdentityAccessor_t = typename IdentityAccessor<T>::type;

    /**
     * Returns an instance of the default accessor for the given container
     *
     * @param The container to get the default accessor for
     * @return Instance of an accessor
     */
    template< typename T >
    IdentityAccessor_t<T>
    getIdentityAccessor(const T&){
        return IdentityAccessor_t<T>();
    }

}  // namespace traits
}  // namespace LiFFT
