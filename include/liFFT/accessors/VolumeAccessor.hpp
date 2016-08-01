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

#include "liFFT/c++14_types.hpp"

namespace LiFFT {
namespace accessors {

    /**
     * Accesses a Volume (1D-3D) by using its ()-operator(x, y, z) where x is the fastest varying index
     */
    struct VolumeAccessor
    {
        template<
            class T_Index,
            class T_Data,
            typename T_SFINAE = std::enable_if_t<
                (traits::NumDims<std::decay_t<T_Data>>::value == 1)
            >*
        >
        auto
        operator()(const T_Index& idx, T_Data& data) const
        -> decltype(data(idx[0]))
        {
            return data(idx[0]);
        }

        template<
            class T_Index,
            class T_Data,
            typename T_SFINAE = std::enable_if_t<
                (traits::NumDims<std::decay_t<T_Data>>::value == 2)
            >*
        >
        auto
        operator()(const T_Index& idx, T_Data& data) const
        -> decltype(data(idx[1], idx[0]))
        {
            return data(idx[1], idx[0]);
        }

        template<
            class T_Index,
            class T_Data,
            typename T_SFINAE = std::enable_if_t<
                (traits::NumDims<std::decay_t<T_Data>>::value == 3)
            >*
        >
        auto
        operator()(const T_Index& idx, T_Data& data) const
        -> decltype(data(idx[2], idx[1], idx[0]))
        {
            return data(idx[2], idx[1], idx[0]);
        }
    };

}  // namespace accessors
}  // namespace LiFFT
