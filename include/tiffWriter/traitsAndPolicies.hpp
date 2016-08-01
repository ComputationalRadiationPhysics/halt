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

#include "tiffWriter/image.hpp"
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/traits/IdentityAccessor.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/traits/IsStrided.hpp"
#include "liFFT/accessors/VolumeAccessor.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/types/Vec.hpp"
#include "liFFT/util.hpp"

namespace LiFFT {
namespace traits {

    template< tiffWriter::ImageFormat T_imgFormat, class T_Allocator >
    struct NumDims< tiffWriter::Image< T_imgFormat, T_Allocator > >: std::integral_constant<unsigned, 2>{};

    template< tiffWriter::ImageFormat T_imgFormat, class T_Allocator >
    struct IsComplex< tiffWriter::Image< T_imgFormat, T_Allocator > >: BoolConst<false>{};

    template< tiffWriter::ImageFormat T_imgFormat, class T_Allocator >
    struct IsStrided< tiffWriter::Image< T_imgFormat, T_Allocator > >: BoolConst<false>{};

    template< tiffWriter::ImageFormat T_imgFormat, class T_Allocator >
    struct IdentityAccessor< tiffWriter::Image< T_imgFormat, T_Allocator > >
    {
        using type = accessors::VolumeAccessor;
    };

}  // namespace traits

namespace policies {

    template< tiffWriter::ImageFormat T_imgFormat, class T_Allocator >
    struct GetExtentsImpl< tiffWriter::Image< T_imgFormat, T_Allocator > >
    {
        using type = tiffWriter::Image< T_imgFormat, T_Allocator >;

        GetExtentsImpl(const type& data): m_extents(data.getHeight(), data.getWidth()){}

        unsigned
        operator[](unsigned dim) const
        {
            return m_extents[dim];
        }
    private:
        const types::Vec<2> m_extents;
    };

}  // namespace policies
}  // namespace LiFFT
