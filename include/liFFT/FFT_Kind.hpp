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

#include <type_traits>

namespace LiFFT {

    /**
     * Enum to define the type/kind of the FFT
     */
    enum class FFT_Kind
    {
        Complex2Complex,
        Real2Complex,
        Complex2Real
    };

    namespace traits {

        /**
         * Trait to specify whether the input is complex
         */
        template< FFT_Kind T_Type >
        struct IsComplexInput: std::true_type{};

        template<>
        struct IsComplexInput< FFT_Kind::Real2Complex >: std::false_type{};

        /**
         * Trait to specify whether the output is complex
         */
        template< FFT_Kind T_Type >
        struct IsComplexOutput: std::true_type{};

        template<>
        struct IsComplexOutput< FFT_Kind::Complex2Real>: std::false_type{};

    }  // namespace traits

}  // namespace LiFFT
