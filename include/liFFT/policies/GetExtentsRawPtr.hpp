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

#include <boost/utility.hpp>
#include <array>
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/c++14_types.hpp"

namespace LiFFT {
namespace policies {

    /**
     * Default implementation when an internal contiguous array has to be allocated
     */
    template< typename T_Data, bool T_copy = true >
    struct GetExtentsRawPtrImpl: private boost::noncopyable
    {
        using Data = T_Data;
        static constexpr unsigned numDims = traits::NumDims<T_Data>::value;

        GetExtentsRawPtrImpl(Data& data){
            GetExtents< Data > extents(data);
            for(unsigned i=0; i<numDims; ++i)
                m_extents[i] = extents[i];
        }

        const unsigned* operator()() const
        {
            return m_extents.data();
        }
    private:
        std::array< unsigned, numDims > m_extents;
    };

    /**
     * Partial specialization when we already have a contiguous array
     */
    template< typename T_Data >
    struct GetExtentsRawPtrImpl< T_Data, false >
    {
        using Data = T_Data;

        GetExtentsRawPtrImpl(Data& data): m_value(data.extents.data()){}

        const unsigned* operator()() const
        {
            return m_value;
        }
    private:
        unsigned* m_value;
    };

    /**
     * Functor that returns a raw ptr to an unsigned int array
     * containing 1 entry per dimension with the extents in that dimensions
     * If a custom numDims value is specified only the last n dimensions are considered
     */
    template< typename T_Data, class T_SFINAE = void >
    struct GetExtentsRawPtr: GetExtentsRawPtrImpl< T_Data, true >{
        using Data = T_Data;
        using Parent = GetExtentsRawPtrImpl< T_Data, true >;

        using Parent::Parent;
    };

    /**
     * Specialization when we have an extents member with a data() function returning a pointer
     */
    template< typename T_Data >
    struct GetExtentsRawPtr<
        T_Data,
        std::enable_if_t<
            std::is_pointer<
                decltype(
                    std::declval<T_Data>().extents.data()
                )
            >::value
        >
    >: GetExtentsRawPtrImpl< T_Data, false >{
        using Data = T_Data;
        using Parent = GetExtentsRawPtrImpl< T_Data, false >;

        using Parent::Parent;
    };

}  // namespace policies
}  // namespace LiFFT
