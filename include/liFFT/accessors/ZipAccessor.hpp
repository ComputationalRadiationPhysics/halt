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

#include "liFFT/traits/IdentityAccessor.hpp"

namespace LiFFT {
namespace accessors {

    /***
     * Accessor that zips together 2 datasets
     * That is an access to (idx, data) will yield Func(AccFirst(idx, dataFirst), AccSecond(idx, data))
     * Therefore you MUST make sure, that the dimensions of dataFirst are at least as big as data!
     */
    template< class T_DataFirst, class T_Func, class T_AccSecond, class T_AccFirst = traits::IdentityAccessor_t<T_DataFirst> >
    struct ZipAccessor
    {
        using AccFirst = T_AccFirst;
        using AccSecond = T_AccSecond;
        using DataFirst= T_DataFirst;
        using Func = T_Func;

        DataFirst& m_dataFirst;
        Func m_func;
        AccSecond m_accSecond;
        AccFirst m_accFirst;

        ZipAccessor(DataFirst& dataFirst, const Func& func = Func(), const AccSecond& accSecond = AccSecond(), const AccFirst& accFirst = AccFirst()):
            m_dataFirst(dataFirst), m_func(func), m_accSecond(accSecond), m_accFirst(accFirst)
        {}

        template< typename T_Idx, class T_Data >
        auto
        operator()(T_Idx& idx, T_Data& data)
        -> decltype( m_func( m_accFirst(idx, m_dataFirst), m_accSecond(idx, data) ) )
        {
            return m_func( m_accFirst(idx, m_dataFirst), m_accSecond(idx, data) );
        }
    };

    template< class T_DataFirst, class T_Func, class T_AccSecond, class T_AccFirst = traits::IdentityAccessor_t<T_DataFirst> >
    ZipAccessor< T_DataFirst, T_Func, T_AccSecond, T_AccFirst>
    makeZipAccessor(T_DataFirst& dataFirst, const T_Func& func, const T_AccSecond& accSecond, const T_AccFirst& accFirst = T_AccFirst())
    {
        return ZipAccessor< T_DataFirst, T_Func, T_AccSecond, T_AccFirst>(dataFirst, func, accSecond, accFirst);
    }


}  // namespace accessors
}  // namespace LiFFT
