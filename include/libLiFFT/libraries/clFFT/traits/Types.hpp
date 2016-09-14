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
    template< int T_NumDims >
    struct Dims: std::integral_constant<clfftDim, CLFFT_1D>{};
    template<>
    struct Dims<1>: std::integral_constant<clfftDim, CLFFT_1D>{};
    template<>
    struct Dims<2>: std::integral_constant<clfftDim, CLFFT_2D>{};
    template<>
    struct Dims<3>: std::integral_constant<clfftDim, CLFFT_3D>{};
}  // namespace traits
}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
