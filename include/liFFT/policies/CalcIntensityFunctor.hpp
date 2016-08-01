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
namespace policies {

    struct CalcIntensityFunc
    {
        template< typename T, typename = std::enable_if_t< LiFFT::traits::IsComplex<T>::value > >
        auto
        operator()(const T& val) const
        -> decltype(val.real*val.real + val.imag*val.imag)
        {
            return val.real*val.real + val.imag*val.imag;
        }

        template< typename T, typename = std::enable_if_t< LiFFT::traits::IsComplex<T>::value > >
        auto
        operator()(const T& val) const
        -> decltype(val[0]*val[0] + val[1]*val[1])
        {
            return val[0]*val[0] + val[1]*val[1];
        }

        template< typename T, typename = std::enable_if_t< !LiFFT::traits::IsComplex<T>::value > >
        auto
        operator()(const T& val) const
        -> decltype(val*val)
        {
            return val*val;
        }
    };

}  // namespace policies
}  // namespace LiFFT
