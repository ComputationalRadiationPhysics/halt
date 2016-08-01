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

#include <functional>
#include "liFFT/types/Vec.hpp"
#include "liFFT/Composite.hpp"

namespace LiFFT {
namespace accessors {

    /**
     * Enumeration of all possible Color(Channel)s
     */
    enum class Color
    {
        Red, Green, Blue, Alpha
    };

    template< Color T_Color >
    struct ShiftRGBA;

    template<>
    struct ShiftRGBA<Color::Red>  : std::integral_constant< unsigned, 0 >{};
    template<>
    struct ShiftRGBA<Color::Green>: std::integral_constant< unsigned, 8 >{};
    template<>
    struct ShiftRGBA<Color::Blue> : std::integral_constant< unsigned, 16 >{};
    template<>
    struct ShiftRGBA<Color::Alpha>: std::integral_constant< unsigned, 24 >{};

    /**
     * Functor that selects only one of the 4 colors by taking 8 of the 32 bit from the input
     * \tparam T_Color The Color-Enum Entry that defines which channel to choose
     */
    template< Color T_Color >
    struct ColorSelectorRGBA
    {

        static constexpr unsigned shift = ShiftRGBA< T_Color >::value;

        unsigned
        operator()(const unsigned value) const
        {
            return (value >> shift) & 0xff;
        }
    };

    /**
     * Functor that maps an 8 Bit color to a floating point value
     */
    template< typename T = float >
    struct ColorMapperByte2FloatingPoint
    {
        using type = unsigned;
        using result_type = T;

        result_type
        operator()(const type value) const
        {
            return static_cast<result_type>(value)/255;
        }
    };

    /**
     * Accessor functor that returns the value at the specified position in the image
     * \tparam T_Image Image class to use, should provide an operator(x,y) to access the pixel value at x,y
     */
    struct ImageAccessor
    {
        using IdxType = types::Vec<2>;

        template< class T_Image >
        unsigned
        operator()(const IdxType& idx, T_Image& img) const
        {
            return img(idx[1], idx[0]);
        }
    };

    template< typename T_FpType = float, Color T_Clr = Color::Red >
    using ImageAccessorGetColorAsFp = Composite<
            ColorMapperByte2FloatingPoint<T_FpType>,
            ColorSelectorRGBA<T_Clr>,
            ImageAccessor
            >;

}  // namespace accessors
}  // namespace LiFFT
