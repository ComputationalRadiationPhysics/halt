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

#include "liFFT/traits/IntegralType.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/traits/IsAoS.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/policies/GetStrides.hpp"
#include "liFFT/accessors/DataContainerAccessor.hpp"
#include "liFFT/policies/CalcIntensity.hpp"

namespace LiFFT {

    template< class T_Input >
    std::enable_if_t< (traits::IsComplex<T_Input>::value && traits::IsAoS<T_Input>::value) >
    calcIntensity(T_Input& input, typename traits::IntegralType<T_Input>::type* output)
    {
        policies::GetExtents<T_Input> extents(input);
        policies::GetStrides<T_Input> strides(input);
        for(unsigned i=0; i<extents[0]; ++i){
            unsigned idx = i*strides[0];
            auto real = input.data[idx].real;
            auto imag = input.data[idx].imag;
            output[i] = real*real+imag*imag;
        }
    }

    template< class T_Input >
    std::enable_if_t< (traits::IsComplex<T_Input>::value && !traits::IsAoS<T_Input>::value) >
    calcIntensity(T_Input& input, typename traits::IntegralType<T_Input>::type* output)
    {
        policies::GetExtents<T_Input> extents(input);
        policies::GetStrides<T_Input> strides(input);
        for(unsigned i=0; i<extents[0]; ++i){
            unsigned idx = i*strides[0];
            auto real = input.data.getRealData()[idx];
            auto imag = input.data.getImagData()[idx];
            output[i] = real*real+imag*imag;
        }
    }

    template< class T_Input >
    std::enable_if_t< !traits::IsComplex<T_Input>::value >
    calcIntensity(T_Input& input, typename traits::IntegralType<T_Input>::type* output)
    {
        policies::GetExtents<T_Input> extents(input);
        policies::GetStrides<T_Input> strides(input);
        for(unsigned i=0; i<extents[0]; ++i){
            unsigned idx = i*strides[0];
            auto real = input.data[idx];
            output[i] = real*real;
        }
    }

    template< class T_Input >
    void
    calcIntensity2(T_Input& input, typename traits::IntegralType<T_Input>::type* output)
    {
        policies::CalcIntensity< accessors::DataContainerAccessor<> > calcIntensity;
        policies::GetExtents<T_Input> extents(input);
        policies::GetStrides<T_Input> strides(input);
        for(unsigned i=0; i<extents[0]; ++i){
            unsigned idx = i*strides[0];
            output[i] = calcIntensity(input, idx);
        }
    }



}  // namespace LiFFT
