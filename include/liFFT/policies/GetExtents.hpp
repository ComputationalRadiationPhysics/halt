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
#include "liFFT/traits/NumDims.hpp"

namespace LiFFT {
namespace policies {

    /**
     * Provides a []-operator to get the extents in the specified dimension of the data object given in the constructor
     * Row-Major order assumed, that is last dimension varies fastest
     */
    template< typename T_Data, typename T_SFINAE = void >
    struct GetExtentsImpl: private boost::noncopyable
    {
        using Data = T_Data;
        static constexpr unsigned numDims = traits::NumDims<Data>::value;

        GetExtentsImpl(const Data& data): m_data(data){}

        unsigned operator[](unsigned dimIdx) const
        {
            return m_data.extents[dimIdx];
        }
    protected:
        const Data& m_data;
    };

    template< typename T_Data >
    struct GetExtentsImpl< T_Data, void_t< decltype(&T_Data::getExtents) > >
    {
        using Data = T_Data;
        static constexpr unsigned numDims = traits::NumDims<Data>::value;
        GetExtentsImpl(const Data& data): m_data(data){}

        unsigned operator[](unsigned dimIdx) const
        {
            return m_data.getExtents()[dimIdx];
        }
    protected:
        const Data& m_data;
    };

    template< typename T_Data >
    struct GetExtents: GetExtentsImpl< T_Data >
    {
        using Parent = GetExtentsImpl< T_Data >;
        using Parent::Parent;
    };

    template< typename T_Data >
    struct GetExtents< const T_Data>: GetExtents< T_Data >
    {
        using Parent = GetExtents< T_Data >;
        using Parent::Parent;
    };

    /**
     * Checks the size of an array against a required size
     * Useful for bounds checking: assert(idx, extents)
     *
     * @param requiredSize Size that is required/Idx that is accessed
     * @param isSize Size that is there / Current extents
     * @return False if isSize[i] < requiredSize[i] for any i in [0, NumDims(requiredSize)), true otherwise
     */
    template< class T_Req, class T_Is >
    std::enable_if_t< !std::is_integral<T_Is>::value, bool >
    checkSizes(const T_Req& requiredSize, const T_Is& isSize)
    {
        static constexpr unsigned numDimsReq = traits::NumDims<T_Req>::value;
        static constexpr unsigned numDimsIs = traits::NumDims<T_Is>::value;
        static_assert(numDimsReq <= numDimsIs, "To few dimensions");
        for(unsigned i=0; i<numDimsReq; ++i)
            if(isSize[i] < requiredSize[i])
                return false;
        return true;
    }

    /**
     * Checks the size against a required size
     * Useful for bounds checking: assert(idx, extents)
     *
     * @param requiredSize Size that is required/Idx that is accessed
     * @param isSize Size that is there / Current extents
     * @return False if requiredSize < isSize or requiredSize has multiple dimensions, true otherwise
     */
    template< class T_Req, class T_Is >
    std::enable_if_t< std::is_integral<T_Is>::value, bool >
    checkSizes(const T_Req& requiredSize, const T_Is& isSize)
    {
        static constexpr unsigned numDimsReq = traits::NumDims<T_Req>::value;
        static_assert(numDimsReq == 1, "To few dimensions");
        if(isSize < requiredSize)
            return false;
        return true;
    }

}  // namespace policies
}  // namespace LiFFT
