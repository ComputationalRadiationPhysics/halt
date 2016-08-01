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

#include "liFFT/policies/flattenIdx.hpp"
#include "liFFT/c++14_types.hpp"
#include "liFFT/util.hpp"

namespace LiFFT {
namespace accessors {

    /**
     * Accessor that can read and write a DataContainer (type with an array-like member named data)
     *
     * \tparam T_isFlatMemory if true, the idx is flattened on access
     */
    template< bool T_isFlatMemory = true >
    struct DataContainerAccessor
    {
    public:
        template< class T_Index, class T_Data >
        auto
        operator()(T_Index&& idx, T_Data& data) const
        -> decltype(
                std::declval< typename T_Data::BaseAccessor >()(
                        0, getConstCorrect<T_Data>(data.m_data)
                )
           )
        {
            auto flatIdx = policies::flattenIdx(idx, data);
            typename T_Data::BaseAccessor acc;
            return acc(flatIdx, getConstCorrect<T_Data>(data.m_data));
        }

        template< class T_Index, class T_Data, typename T_Value >
        void
        operator()(T_Index&& idx, T_Data& data, T_Value&& value) const
        {
            auto flatIdx = policies::flattenIdx(idx, data);
            typename T_Data::BaseAccessor acc;
            acc(flatIdx, getConstCorrect<T_Data>(data.m_data), std::forward<T_Value>(value));
        }
    };

    template<>
    struct DataContainerAccessor<false>
    {
    public:
        template< class T_Index, class T_Data >
        auto
        operator()(T_Index&& idx, T_Data& data) const
        -> decltype(
                std::declval< typename T_Data::BaseAccessor >()(
                        idx, getConstCorrect<T_Data>(data.m_data)
                )
           )
        {
            typename T_Data::BaseAccessor acc;
            return acc(idx, getConstCorrect<T_Data>(data.m_data));
        }

        template< class T_Index, class T_Data, typename T_Value >
        void
        operator()(T_Index&& idx, T_Data& data, T_Value&& value) const
        {
            typename T_Data::BaseAccessor acc;
            acc(idx, getConstCorrect<T_Data>(data.m_data), std::forward<T_Value>(value));
        }
    };

}  // namespace accessors
}  // namespace LiFFT
