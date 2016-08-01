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
#include "liFFT/traits/IntegralType.hpp"
#include <type_traits>

namespace LiFFT {
namespace policies {

    namespace detail
    {
        template< class T, bool T_isComplex = traits::IsComplex<T>::value >
        struct CalcIntensityImpl
        {
            using RetType = typename traits::IntegralType<T>::type;

            RetType
            operator()(const T& value)
            {
                RetType r = value.real;
                RetType i = value.imag;
                return r*r+i*i;
            }
        };

        template< class T >
        struct CalcIntensityImpl< T, false >
        {
            using RetType = typename traits::IntegralType<T>::type;

            RetType
            operator()(const RetType value)
            {
                return value*value;
            }
        };


        template< class T_Accessor >
        struct CalcIntensity
        {
            template< typename T_Data >
            auto
            operator()(const T_Data& data, unsigned idx)
            -> typename traits::IntegralType< std::result_of_t< T_Accessor(unsigned, const T_Data&) > >::type
            {
                T_Accessor accessor;
                CalcIntensityImpl< std::result_of_t< T_Accessor(unsigned, const T_Data&) > > intensity;
                return intensity(accessor(idx, data));
            }
        };

    }//namespace detail

    template< class T_Accessor >
    struct CalcIntensity : public detail::CalcIntensity<T_Accessor>{};

}  // namespace policies
}  // namespace LiFFT
