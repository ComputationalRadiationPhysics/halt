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

#include "libLiFFT/policies/GetNumElements.hpp"

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

    /**
     * Getting memory size for inplace transforms is a bit tricky. Therefore use this trait
     */
    template< typename T_Precision,
              bool T_isComplexIn,
              bool T_isComplexOut,
              unsigned T_numDims >
    struct GetInplaceMemSize
    {
        using Precision = T_Precision;
        static constexpr bool isComplexIn = T_isComplexIn;
        static constexpr bool isComplexOut = T_isComplexOut;
        static constexpr unsigned numDims = T_numDims;

        template< class T_Extents >
        static size_t
        get(const T_Extents& fullExtents)
        {
          auto extents(fullExtents);
          if(!isComplexIn || !isComplexOut)
          {
            extents[numDims - 1] = extents[numDims - 1] / 2 + 1;
          }
          // Get number of complex elements
          unsigned numElements = LiFFT::policies::getNumElementsFromExtents(extents);
          return numElements * 2 * sizeof(T_Precision);
        }
    };

}  // namespace policies
}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
