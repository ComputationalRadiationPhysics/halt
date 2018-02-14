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

#include <clFFT.h>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace traits{

  template< typename T_Precision >
  struct FFTPrecision: std::integral_constant< clfftPrecision, CLFFT_SINGLE >{};
  template<>
  struct FFTPrecision<float>: std::integral_constant< clfftPrecision, CLFFT_SINGLE >{};
  template<>
  struct FFTPrecision<double>: std::integral_constant< clfftPrecision, CLFFT_DOUBLE >{};

  template< bool T_IsComplex, bool T_IsOtherComplex >
  struct FFTLayout: std::integral_constant< clfftLayout, CLFFT_COMPLEX_INTERLEAVED >{};
  template<>
  struct FFTLayout<true,true>: std::integral_constant< clfftLayout, CLFFT_COMPLEX_INTERLEAVED >{};
  /* R2C/C2R only uses Hermitian layout */
  template<>
  struct FFTLayout<true,false>: std::integral_constant< clfftLayout,  CLFFT_HERMITIAN_INTERLEAVED  >{};
  template<>
  struct FFTLayout<false,true>: std::integral_constant< clfftLayout, CLFFT_REAL >{};

  template< bool T_isInplace >
  struct FFTInplace: std::integral_constant< clfftResultLocation, CLFFT_INPLACE >{};
  template<>
  struct FFTInplace<true>: std::integral_constant< clfftResultLocation, CLFFT_INPLACE >{};
  template<>
  struct FFTInplace<false>: std::integral_constant< clfftResultLocation, CLFFT_OUTOFPLACE >{};

}  // namespace traits
}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
