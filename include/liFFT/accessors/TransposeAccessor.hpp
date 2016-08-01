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

#include "liFFT/accessors/ArrayAccessor.hpp"

namespace LiFFT {
namespace accessors {

    /**
     * Accessor that transposes all accesses such that the 2nd half comes before the first
     */
    template< class T_BaseAccessor = ArrayAccessor<true> >
    struct TransposeAccessor
    {
    private:
        using BaseAccessor = T_BaseAccessor;
        BaseAccessor m_acc;

        template< class T_Index, class T_Data >
        void
        transposeIdx(const T_Index& idxIn, T_Index& idxOut, const T_Data& data) const
        {
            static constexpr unsigned numDims = traits::NumDims< T_Data >::value;
            policies::GetExtents< T_Data > extents(data);
            for(unsigned i=0; i<numDims; ++i)
            {
                unsigned ext = extents[i];
                unsigned idx = idxIn[i];
                idxOut[i] = (idx >= ext/2) ? idx - ext/2 : idx + ext/2;
            }
        }
    public:

        explicit TransposeAccessor(const BaseAccessor& acc = BaseAccessor()): m_acc(acc){}

        template< class T_Index, class T_Data >
        auto
        operator()(const T_Index& idx, T_Data& data) const
        -> decltype(m_acc(idx, data))
        {
            T_Index transposedIdx;
            transposeIdx(idx, transposedIdx, data);
            return m_acc(transposedIdx, data);
        }

        template< class T_Index, class T_Data, typename T_Value >
        auto
        operator()(const T_Index& idx, T_Data& data, T_Value&& value)
        -> decltype( m_acc(idx, data, std::forward<T_Value>(value)) )
        {
            T_Index transposedIdx;
            transposeIdx(idx, transposedIdx, data);
            m_acc(transposedIdx, data, std::forward<T_Value>(value));
        }
    };

    /**
     * Creates a transpose accessor for the given accessor
     * This transposes all accesses such that the 2nd half comes before the first
     *
     * @param acc Base accessor
     * @return TransposeAccessor
     */
    template< class T_BaseAccessor >
    TransposeAccessor< T_BaseAccessor >
    makeTransposeAccessor(T_BaseAccessor&& acc)
    {
        return TransposeAccessor< T_BaseAccessor >(std::forward<T_BaseAccessor>(acc));
    }

    /**
     * Creates a transpose accessor for the given container using its default accessor
     *
     * @param Container instance
     * @return TransposeAccessor
     */
    template< class T>
    TransposeAccessor< traits::IdentityAccessor_t<T> >
    makeTransposeAccessorFor(const T& = T())
    {
        return TransposeAccessor< traits::IdentityAccessor_t<T> >();
    }

}  // namespace accessors
}  // namespace LiFFT
