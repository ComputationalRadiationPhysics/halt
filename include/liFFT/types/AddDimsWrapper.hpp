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

#include <utility>

namespace LiFFT {
namespace types {

    /**
     * Wrapper used to add dimensionality Information to a class without using traits
     *
     * \tparam T_Base Base class to wrap
     * \tparam T_numDims number of dimensions this class should have
     */
    template< class T_Base, unsigned T_numDims >
    struct AddDimsWrapper: T_Base
    {
        using Base = T_Base;
        using Base::Base;
        static constexpr unsigned numDims = T_numDims;

        explicit AddDimsWrapper(const Base& base): Base(base){}
        explicit AddDimsWrapper(Base&& base): Base(std::move(base)){}
    };

}  // namespace types
}  // namespace LiFFT
