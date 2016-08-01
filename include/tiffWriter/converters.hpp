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
#include <limits>
#include <algorithm>
#include <array>
#include "liFFT/c++14_types.hpp"

namespace tiffWriter {

    /**
     * Gets the maximum value (in an image) of the given type
     * Evaluates to numeric_limits::max() for integral types and 1 for floats
     */
    template< typename T, bool T_isFloat = std::is_floating_point<T>::value >
    struct GetMaxVal: std::integral_constant< T, std::numeric_limits<T>::max() >{};

    template< typename T>
    struct GetMaxVal< T, true >: std::integral_constant< int, 1 >{};

    /**
     * Converts a single channel
     * FP -> FP: values are unchanged
     */
    template<
        typename T_Src,
        typename T_Dest,
        bool T_minIsBlack,
        bool T_srcIsFloat = std::is_floating_point<T_Src>::value,
        bool T_destIsFloat = std::is_floating_point<T_Dest>::value
    >
    struct ConvertChannel
    {
        using Src = T_Src;
        using Dest = T_Dest;
        static constexpr bool minIsBlack = T_minIsBlack;

        static_assert(std::is_floating_point<Src>::value, "Invalid src type");
        static_assert(std::is_floating_point<Dest>::value, "Invalid dst type");

        Dest
        operator()(Src src)
        {
            return (minIsBlack) ? src : GetMaxVal<Src>::value - src;
        }
    };

    /**
     * Integral -> FP: normalize to [0, 1]
     */
    template< typename T_Src, typename T_Dest, bool T_minIsBlack >
    struct ConvertChannel< T_Src, T_Dest, T_minIsBlack, false, true >
    {
        using Src = T_Src;
        using Dest = T_Dest;
        static constexpr bool minIsBlack = T_minIsBlack;

        static_assert(std::is_integral<Src>::value, "Invalid src type");
        static_assert(std::is_floating_point<Dest>::value, "Invalid dst type");


        static constexpr Dest divisor = Dest(std::numeric_limits<Src>::max()) - std::numeric_limits<Src>::min();
        static constexpr Dest factor = 1/divisor;

        static constexpr Dest min = std::numeric_limits<Src>::min();

        Dest
        operator()(Src src)
        {
            Dest res = (src - min) * factor;
            return (minIsBlack) ? res : GetMaxVal<Src>::value - res;
        }
    };

    /**
     * Integral -> Integral: Extent from one range to another
     */
    template< typename T_Src, typename T_Dest, bool T_minIsBlack >
    struct ConvertChannel< T_Src, T_Dest, T_minIsBlack, false, false >
    {
        using Src = T_Src;
        using Dest = T_Dest;
        static constexpr bool minIsBlack = T_minIsBlack;

        static_assert(std::is_integral<Src>::value, "Invalid src type");
        static_assert(std::is_integral<Dest>::value, "Invalid dst type");

        using USrc = std::make_unsigned_t<Src>;
        // Equivalent to: std::numeric_limits<Src>::max() - std::numeric_limits<Src>::min()
        static constexpr USrc divisor = std::numeric_limits<USrc>::max();

        static constexpr Dest max = std::numeric_limits<Dest>::max();
        static constexpr Dest min = std::numeric_limits<Dest>::min();

        Dest
        operator()(Src src)
        {
            Dest res;
            if(sizeof(Dest) > sizeof(Src))
                res = static_cast<Dest>(src * (max / divisor) - min);
            else
                res = static_cast<Dest>(src / (divisor/max) - min);
            return (minIsBlack) ? res : GetMaxVal<Src>::value - res;
        }
    };

    /**
     * FP -> Integral: Assume normalized FP [0,1] and scale accordingly
     */
    template< typename T_Src, typename T_Dest, bool T_minIsBlack >
    struct ConvertChannel< T_Src, T_Dest, T_minIsBlack, true, false >
    {
        using Src = T_Src;
        using Dest = T_Dest;
        static constexpr bool minIsBlack = T_minIsBlack;

        static_assert(std::is_floating_point<Src>::value, "Invalid src type");
        static_assert(std::is_integral<Dest>::value, "Invalid dst type");

        static constexpr Dest max = std::numeric_limits<Dest>::max();
        static constexpr Dest min = std::numeric_limits<Dest>::min();

        Dest
        operator()(Src src)
        {
            Dest res = static_cast<Dest>(src * max - min);
            return (minIsBlack) ? res : GetMaxVal<Src>::value - res;
        }
    };

    /**
     * Converts an (A)RGB value to mono using the weighted sum method
     */
    template< typename T_Src, uint16_t T_numChannels, typename T_Dest, bool T_minIsBlack >
    struct ConvertARGBToMono
    {
        using Src = T_Src;
        static constexpr uint16_t numChannels = T_numChannels;
        static constexpr bool minIsBlack = T_minIsBlack;

        static_assert(numChannels == 3 || numChannels == 4, "Only works for (A)RGB");
        using Array = std::array<Src, numChannels>;

        using FloatType =
                std::conditional_t<
                    std::is_same<T_Src, double>::value || std::is_same<T_Dest, double>::value,
                    double,
                    float
                >;

        static constexpr FloatType rWeight = 0.299;
        static constexpr FloatType gWeight = 0.587;
        static constexpr FloatType bWeight = 0.114;

        T_Dest
        operator()(const Array& channels)
        {
            // Respect endianess: If Red is the lowest Byte it has the lowest offset
            T_Dest res = static_cast<T_Dest>(rWeight * channels[0] + gWeight * channels[1] + bWeight * channels[2]);
            return (minIsBlack) ? res : GetMaxVal<T_Dest>::value - res;
        }
    };

    /**
     * Converts a reference to a composite element into an array of its elements and passes it to the function
     */
    template< typename T_El, uint16_t T_numElSrc, class T_Func, uint16_t T_numElDest = T_numElSrc >
    struct AccessChannels
    {
        using El = T_El;
        static constexpr uint16_t numElSrc = T_numElSrc;
        static constexpr uint16_t numElDest = T_numElDest;
        using Func = T_Func;

        static_assert( numElSrc == numElDest ||
                        (numElSrc == 4 && numElDest == 3) ||
                        (numElSrc == 3 && numElDest == 4),
                        "Can only copy or convert from (A)RGB to (A)RGB");

        Func m_func;
        using Dest = typename Func::Src;
        using Array = std::array<Dest, numElDest>;

        template< typename T_Src >
        auto
        operator()(const T_Src& src)
        -> std::result_of_t<Func(Array)>
        {
            static constexpr uint16_t numCopy = (numElSrc < numElDest) ? numElSrc : numElDest; //std::cmin(numElSrc, numElDest);

            const El* els = reinterpret_cast<const El*>(&src);
            Array tmp;
            std::copy_n(els, numCopy, tmp.begin());
            if(numCopy < numElDest)
            {
                // Set alpha value to max
                tmp[numCopy] = GetMaxVal<Dest>::value;
            }
            return m_func(tmp);
        }
    };

    /**
     * Applies a function to all channels
     */
    template< typename T_Src, uint16_t T_numChannels, class T_Func >
    struct ConvertAllChannels
    {
        using Src = T_Src;
        static constexpr uint16_t numChannels = T_numChannels;

        using Dest = std::result_of_t<T_Func(T_Src)>;
        using Channels = std::array<Src, numChannels>;
        using Result = std::array<Dest, numChannels>;
        T_Func m_func;

        Result
        operator()(const Channels& channels)
        {
            Result res;
            std::transform(channels.begin(), channels.end(), res.begin(), m_func);
            return res;
        }
    };

    template< typename T_Src, typename T_Dest, uint16_t T_numChannelsSrc, uint16_t T_numChannelsDest, bool T_minIsBlack >
    struct Convert:
        AccessChannels<
            T_Src,
            T_numChannelsSrc,
            ConvertAllChannels<
                T_Src,
                T_numChannelsDest,
                ConvertChannel<T_Src, T_Dest, T_minIsBlack>
            >,
            T_numChannelsDest
        >
    {
        using Src = T_Src;
        using Dest = T_Dest;
    };

    template< typename T_Src, typename T_Dest, bool T_minIsBlack >
    struct Convert< T_Src, T_Dest, 1, 1, T_minIsBlack >: ConvertChannel<T_Src, T_Dest, T_minIsBlack>{};

    template< typename T_Src, typename T_Dest, uint16_t T_numChannelsDest, bool T_minIsBlack >
    struct Convert< T_Src, T_Dest, 1, T_numChannelsDest, T_minIsBlack >
    {
        using Src = T_Src;
        using Dest = T_Dest;
        using Result = std::array<Dest, T_numChannelsDest>;
        ConvertChannel<Src, Dest, T_minIsBlack> m_conv;

        Result
        operator()(Src src)
        {
            Result res;
            res.fill(m_conv(src));
            return res;
        }
    };

    template< typename T_Src, typename T_Dest, uint16_t T_numChannelsSrc, bool T_minIsBlack >
    struct Convert< T_Src, T_Dest, T_numChannelsSrc, 1, T_minIsBlack >:
        AccessChannels<
            T_Src,
            T_numChannelsSrc,
            ConvertARGBToMono< T_Src, T_numChannelsSrc, T_Dest, T_minIsBlack >
        >
    {
        using Src = T_Src;
        using Dest = T_Dest;
    };

}  // namespace tiffWriter
