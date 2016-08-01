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

namespace LiFFT {

    /**
     * Helper for void_t (Workaround, see Paper from Walter Brown)
     */
    template <typename...>
    struct voider { using type = void; };

    /**
     * void_t for evaluating arguments, then returning void
     * Used for SFINAE evaluation of types
     */
    template <typename... Ts>
    using void_t = typename voider<Ts...>::type;

}  // namespace LiFFT
