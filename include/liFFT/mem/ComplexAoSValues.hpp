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

#include <memory>
#include "liFFT/types/Complex.hpp"
#include "liFFT/mem/AoSValues.hpp"

namespace LiFFT {
namespace mem {

    template< typename T, bool T_ownsPointer = true >
    class ComplexAoSValues: public detail::AoSValues< types::Complex<T>, T_ownsPointer >
    {
    public:
        using Parent = detail::AoSValues< types::Complex<T>, T_ownsPointer >;

        using Parent::Parent;
    };

}  // namespace mem
}  // namespace LiFFT
