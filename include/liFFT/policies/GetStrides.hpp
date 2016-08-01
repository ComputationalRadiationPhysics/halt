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
#include "liFFT/traits/IsStrided.hpp"
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/types/Vec.hpp"

namespace LiFFT {
namespace policies {

    namespace detail {

        /**
         * Default specialization for unstrided types: Use extents
         */
        template< typename T_Data, bool T_isStrided = traits::IsStrided< T_Data >::value >
        struct GetStrides: private boost::noncopyable
        {
            using Data = T_Data;
            static constexpr unsigned numDims = traits::NumDims< Data >::value;

            GetStrides(const Data& data)
            {
                GetExtents< T_Data > extents(data);
                static_assert(numDims >= 1, "No dimensions");
                m_strides[numDims - 1] = 1;
                for(unsigned i=numDims - 1; i>0; --i)
                    m_strides[i-1] = m_strides[i] * extents[i];
            }

            size_t operator[](unsigned dimIdx) const
            {
                return m_strides[dimIdx];
            }
        protected:
            types::Vec<numDims, size_t> m_strides;
        };

        /**
         * Default specialization for strided types: Use strides member
         */
        template< typename T_Data >
        struct GetStrides< T_Data, true >: private boost::noncopyable
        {
            using Data = T_Data;

            GetStrides(const Data& data): m_data(data){}

            size_t operator[](unsigned dimIdx) const
            {
                return m_data.strides[dimIdx];
            }
        protected:
            const Data& m_data;
        };

    }  // namespace detail

    /**
     * Provides a []-operator to get the strides in the specified dimension of the data object given in the constructor
     */
    template< typename T_Data >
    struct GetStrides: detail::GetStrides< T_Data >
    {
        using detail::GetStrides< T_Data >::GetStrides;
    };

}  // namespace policies
}  // namespace LiFFT
