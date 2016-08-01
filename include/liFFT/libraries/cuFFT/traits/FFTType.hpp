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

#include <cufft.h>

namespace LiFFT {
namespace libraries {
namespace cuFFT {
namespace traits{

        /**
         * Defines the (cuFFT)-Type of the FFT
         */
        template< typename T_Precision, bool T_IsComplexIn, bool T_IsComplexOut >
        struct FFTType: std::integral_constant< cufftType, CUFFT_C2C >{};

        template<>
        struct FFTType< float, false, true >: std::integral_constant< cufftType, CUFFT_R2C >{};

        template<>
        struct FFTType< float, true, false >: std::integral_constant< cufftType, CUFFT_C2R >{};

        template<>
        struct FFTType< double, true, true >: std::integral_constant< cufftType, CUFFT_Z2Z >{};

        template<>
        struct FFTType< double, false, true >: std::integral_constant< cufftType, CUFFT_D2Z >{};

        template<>
        struct FFTType< double, true, false >: std::integral_constant< cufftType, CUFFT_Z2D >{};

}  // namespace traits
}  // namespace cuFFT
}  // namespace libraries
}  // namespace LiFFT
