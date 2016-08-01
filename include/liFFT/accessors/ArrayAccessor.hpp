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
namespace accessors {

    /**
     * Accessor that access the data directly via the index
     *
     * \tparam T_isFunctor When false, the array is accessed via []-operator, else via ()-operator
     */
    template< bool T_isFunctor = false >
    struct ArrayAccessor
    {
    public:
        template< class T_Index, class T_Data >
        auto
        operator()(const T_Index& idx, T_Data& data) const
        -> decltype(data[idx])
        {
            return data[idx];
        }

        template< class T_Index, class T_Data, typename T_Value >
        void
        operator()(const T_Index& idx, T_Data& data, T_Value&& value) const
        {
            data[idx] = std::forward<T_Value>(value);
        }
    };

    template<>
    struct ArrayAccessor< true >
    {
    public:
        template< class T_Index, class T_Data >
        auto
        operator()(const T_Index& idx, T_Data& data) const
        -> decltype(data(idx))
        {
            return data(idx);
        }

        template< class T_Index, class T_Data, typename T_Value >
        void
        operator()(const T_Index& idx, T_Data& data, T_Value&& value) const
        {
            data(idx) = std::forward<T_Value>(value);
        }
    };

}  // namespace accessors
}  // namespace LiFFT
