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

#include "liFFT/traits/NumDims.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/types/Vec.hpp"
#include "liFFT/c++14_types.hpp"

namespace LiFFT {
namespace policies {

    namespace detail {

        template<bool T_lastDim>
        struct LoopImpl
        {
            template<
                unsigned T_curDim,
                unsigned T_endDim,
                class T_Index,
                class T_Extents,
                class T_Src,
                class T_SrcAccessor,
                class T_Handler,
                class... T_Args
            >
            static void
            loop(T_Index& idx, const T_Extents& extents, T_Src& src, T_SrcAccessor&& accSrc, T_Handler& handler, T_Args&& ... args)
            {
                for(idx[T_curDim]=0; idx[T_curDim]<extents[T_curDim]; ++idx[T_curDim])
                {
                    handler.template handleInnerLoop<T_curDim>(idx, src, accSrc, std::forward<T_Args>(args)...);
                }
            }
        };

        template<>
        struct LoopImpl<false>
        {
            template<
                unsigned T_curDim,
                unsigned T_endDim,
                class T_Index,
                class T_Extents,
                class T_Src,
                class T_SrcAccessor,
                class T_Handler,
                class... T_Args
            >
            static void
            loop(T_Index& idx, const T_Extents& extents, T_Src& src, T_SrcAccessor&& accSrc, T_Handler& handler, T_Args&& ... args)
            {
                for(idx[T_curDim]=0; idx[T_curDim]<extents[T_curDim]; ++idx[T_curDim])
                {
                    handler.template handleLoopPre< T_curDim, T_endDim >(idx, src, accSrc, std::forward<T_Args>(args)...);
                    LoopImpl< (T_curDim+2 == T_endDim) >::
                            template loop< T_curDim+1, T_endDim >(idx, extents, src, std::forward<T_SrcAccessor>(accSrc), handler, std::forward<T_Args>(args)...);
                    handler.template handleLoopPost< T_curDim, T_endDim >(idx, src, accSrc, std::forward<T_Args>(args)...);
                }
            }
        };

    }  // namespace detail

    /**
     * Defines a loop over all dimensions of src
     * Expects a handler that is then called for each iteration. Specifically:
     * Before any inner loop:  handler.handleLoopPre< curDim, endDim >(idx, src, accSrc, args...)
     * After any inner loop:   handler.handleLoopPost< curDim, endDim >(idx, src, accSrc, args...)
     * In the most inner loop: handler.handleInnerLoop< curDim >(idx, src, accSrc, args...)
     */
    template<
        class T_Src,
        class T_SrcAccessor = const LiFFT::traits::IdentityAccessor_t<T_Src>
    >
    struct Loop
    {
    public:
        template< class T_Handler, class... T_Args >
        static void
        execute(T_Src& src, T_Handler&& handler, T_SrcAccessor& accSrc, T_Args&& ... args)
        {
            static constexpr unsigned numDims = traits::NumDims<std::remove_const_t<T_Src>>::value;

            using ExtentsVec = types::Vec<numDims>;
            ExtentsVec idx;
            detail::LoopImpl< (numDims == 1) >::
                    template loop< 0, numDims >(idx, GetExtents<T_Src>(src), src, accSrc, handler, std::forward<T_Args>(args)...);
        }
    };

    template<
        class T_Src,
        class T_Handler,
        class T_SrcAccessor = const LiFFT::traits::IdentityAccessor_t<T_Src>,
        class... T_Args
    >
    void
    loop(T_Src& src, T_Handler&& handler, T_SrcAccessor& accSrc, T_Args&& ... args)
    {
        Loop< T_Src, T_SrcAccessor >::execute(src, handler, accSrc, std::forward<T_Args>(args)...);
    }

    template<
        class T_Src,
        class T_Handler,
        class T_SrcAccessor = const LiFFT::traits::IdentityAccessor_t<T_Src>
    >
    void
    loop(T_Src& src, T_Handler&& handler, T_SrcAccessor& accSrc = T_SrcAccessor())
    {
        Loop< T_Src, T_SrcAccessor >::execute(src, handler, accSrc);
    }

}  // namespace policies
}  // namespace LiFFT
