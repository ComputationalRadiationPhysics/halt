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

#include "liFFT/void_t.hpp"
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/types/Vec.hpp"
#include "liFFT/c++14_types.hpp"
#include <type_traits>

namespace LiFFT {
namespace traits {

    template<
        class T_Accessor,
        typename T_Data,
        typename T_Index = types::Vec< NumDims<T_Data>::value >,
        typename T_SFINAE = void
    >
    struct IsReadAccessor: std::false_type{};

    template< class T_Accessor, typename T_Data, typename T_Index>
    struct IsReadAccessor< T_Accessor, T_Data, T_Index, void_t<
            std::result_of_t<T_Accessor(T_Index, T_Data)>
        >
    >: std::true_type{};

    template<
        class T_Accessor,
        typename T_Data,
        typename T_Value,
        typename T_Index = types::Vec< NumDims<T_Data>::value >,
        typename T_SFINAE = void
    >
    struct IsWriteAccessor: std::false_type{};

    template< class T_Accessor, typename T_Data, typename T_Value, typename T_Index>
    struct IsWriteAccessor< T_Accessor, T_Data, T_Value, T_Index, void_t<
            std::result_of_t<T_Accessor(T_Index, T_Data&, T_Value)>
        >
    >: std::true_type{};


    template<
        class T_Accessor,
        typename T_Data,
        typename T_Value = char,
        typename T_SFINAE = void
    >
    struct IsStreamAccessor: std::false_type{};

    template< class T_Accessor, typename T_Data, typename T_Value>
    struct IsStreamAccessor< T_Accessor, T_Data, T_Value, void_t<
            decltype(&T_Accessor::getDelimiters)
        >
    >: std::true_type
    {
        static_assert(
                IsReadAccessor< T_Accessor, T_Data >::value ||
                IsWriteAccessor< T_Accessor, T_Data, T_Value >::value,
                "Only Accessors should be checked with this trait");
    };

    template<
        class T_Accessor,
        typename T_Data,
        typename T_Index = types::Vec< NumDims<T_Data>::value >
    >
    struct ReadAccessorReturnType
    {
        static_assert(
                IsReadAccessor< T_Accessor, T_Data >::value,
                "Only Read-Accessors should be checked with this trait");

        using type = std::result_of_t<T_Accessor(T_Index, T_Data)>;
    };

}  // namespace traits
}  // namespace LiFFT
