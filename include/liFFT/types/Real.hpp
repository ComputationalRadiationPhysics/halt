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

namespace LiFFT {
namespace types {

    template< typename T >
    struct Real
    {
        using type = T;
        static constexpr bool isComplex = false;
        T value;

        Real(){}
        Real(T val):value(val){}
        operator T() const {return value;}
    };

}  // namespace types

namespace traits {

    template< typename T >
    struct IsBinaryCompatibleImpl< types::Real<T>, T >: std::true_type{};

}  // namespace traits
}  // namespace LiFFT
