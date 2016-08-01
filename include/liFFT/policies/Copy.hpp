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
#include "liFFT/traits/AccessorTraits.hpp"
#include "liFFT/policies/Loop.hpp"
#include "liFFT/c++14_types.hpp"

namespace LiFFT {
namespace policies {
    namespace detail {

        struct CopyHandler
        {
            template<
                unsigned T_curDim,
                class T_Index,
                class T_Src,
                class T_SrcAccessor,
                class T_Dst,
                class T_DstAccessor
                >
            void
            handleInnerLoop(const T_Index& idx, const T_Src& src, T_SrcAccessor&& accSrc, T_Dst& dst, T_DstAccessor&& accDst)
            {
                if(idx[T_curDim] > 0){
                    accSrc.readDelimiter(src, T_curDim);
                    accDst.writeDelimiter(dst, T_curDim);
                }
                accDst.write(idx, dst, accSrc.read(idx, src));
            }

            template<
                unsigned T_curDim,
                unsigned T_endDim,
                class T_Index,
                class T_Src,
                class T_SrcAccessor,
                class T_Dst,
                class T_DstAccessor
                >
            void
            handleLoopPre(const T_Index& idx, const T_Src& src, T_SrcAccessor&& accSrc, T_Dst& dst, T_DstAccessor&& accDst)
            {
                if(idx[T_curDim] > 0){
                    accSrc.readDelimiter(src, T_curDim);
                    accDst.writeDelimiter(dst, T_curDim);
                }
            }

            template<
                unsigned T_curDim,
                unsigned T_endDim,
                class T_Index,
                class T_Src,
                class T_SrcAccessor,
                class T_Dst,
                class T_DstAccessor
                >
            void
            handleLoopPost(
                T_Index const & /*idx*/,
                T_Src const & /*src*/,
                T_SrcAccessor && /*accSrc*/,
                T_Dst const & /*dst*/,
                T_DstAccessor && /*accDst*/ )
            {}
        };

        template< class T_BaseAccessor >
        struct ReadAccessorWrapper
        {
            using BaseAccessor = T_BaseAccessor;

            BaseAccessor m_acc;

            ReadAccessorWrapper(){}
            ReadAccessorWrapper(const BaseAccessor& acc): m_acc(acc){}
            ReadAccessorWrapper(BaseAccessor&& acc): m_acc(std::move(acc)){}

            template< typename T_Data >
            std::enable_if_t< traits::IsStreamAccessor< BaseAccessor, T_Data >::value >
            readDelimiter(T_Data& data, unsigned dim)
            {
                m_acc.skipDelimiter(data, dim);
            }

            template< typename T_Data >
            std::enable_if_t< !traits::IsStreamAccessor< BaseAccessor, T_Data >::value >
            readDelimiter(T_Data& /*data*/, unsigned /*dim*/){}

            template<
                typename T_Index,
                typename T_Data
            >
            auto
            read(const T_Index& idx, T_Data& data)
            -> decltype(m_acc(idx, data))
            {
                return m_acc(idx, data);
            }
        };

        template< class T_BaseAccessor >
        struct WriteAccessorWrapper
        {
            using BaseAccessor = T_BaseAccessor;

            BaseAccessor m_acc;

            WriteAccessorWrapper(){}
            WriteAccessorWrapper(const BaseAccessor& acc): m_acc(acc){}
            WriteAccessorWrapper(BaseAccessor&& acc): m_acc(std::move(acc)){}

            template< typename T_Data >
            std::enable_if_t< traits::IsStreamAccessor< BaseAccessor, T_Data, char >::value >
            writeDelimiter(T_Data& data, unsigned dim)
            {
                types::Vec<1> dummyIdx;
                m_acc(dummyIdx, data, m_acc.getDelimiters()[dim]);
            }

            template< typename T_Data >
            std::enable_if_t< !traits::IsStreamAccessor< BaseAccessor, T_Data, char >::value >
            writeDelimiter(T_Data& /*data*/, unsigned /*dim*/){}

            /**
             * Write method for accessors that have an extra write method, that is a ()-operator with 3 arguments
             *
             * @param idx Index
             * @param data Datacontainer to write to
             * @param value Value to write
             */
            template<
                typename T_Index,
                typename T_Data,
                typename T_Value
            >
            std::enable_if_t< traits::IsWriteAccessor< BaseAccessor, T_Data, T_Value, T_Index >::value >
            write(const T_Index& idx, T_Data& data, T_Value&& value)
            {
                m_acc(idx, data, std::forward<T_Value>(value));
            }

            /**
             * Fallback write method for accessors without write method
             * Tries to use the read method ( ()-operator with 2 arguments) and write to its result
             *
             * @param idx Index
             * @param data Datacontainer to write to
             * @param value Value to write
             */
            template<
                typename T_Index,
                typename T_Data,
                typename T_Value
            >
            std::enable_if_t< !traits::IsWriteAccessor< BaseAccessor, T_Data, T_Value, T_Index >::value >
            write(const T_Index& idx, T_Data& data, T_Value&& value)
            {
                using LeftType = decltype(m_acc(idx, data));
                static_assert(AssertValue<std::is_assignable<LeftType, T_Value>>::value, "Cannot assign value returned from srcAccessor to dstData");
                m_acc(idx, data) = std::forward<T_Value>(value);
            }
        };
    }  // namespace detail


    /**
     * Policy that copies the contents of an array(-like) type to a stream(-like) type
     * That is something that can't be accessed with indices
     *
     * Provides a ()-operator(source, destination)
     *
     * \tparam T_SrcAccessor Accessor used to get an element from src: <type> operator(idx, src)
     * \tparam T_DstAccessor Accessor used to set an element in dst  : operator(idx, dst, value)
     * For Stream accessors:
     *          getDelimiters() -> array like type with delimiters for each dimension
     * For Stream-Read accessors:
     *          skipDelimiter(data, curDim) -> Skips the delimiter of this dimension in the stream
     */
    template< class T_SrcAccessor, class T_DstAccessor >
    struct Copy
    {
    private:
        detail::ReadAccessorWrapper< T_SrcAccessor > m_accSrc;
        detail::WriteAccessorWrapper< T_DstAccessor > m_accDst;

        template< class T_Accessor, unsigned T_numDims, bool T_isStreamAcc = false >
        struct DelimiterDimOk: std::true_type{};

        template< class T_Accessor, unsigned T_numDims >
        struct DelimiterDimOk< T_Accessor, T_numDims, true >
        {
            static constexpr bool value = T_numDims <= traits::NumDims<
                    decltype(
                            std::declval<T_Accessor>().getDelimiters()
                            )
                    >::value;
        };
        template< bool T_doCheck = false, class dummy = void >
        struct ExtentsCheck
        {
            template< class... T >
            static void
            check(T&&...){}
        };

        template<class dummy>
        struct ExtentsCheck<true, dummy>
        {
            template< class T_Src, class T_Dst >
            static void
            check(const T_Src& src, const T_Dst& dst)
            {
                GetExtents<T_Src> extSrc(src);
                GetExtents<T_Dst> extDst(dst);
                for(unsigned i=0; i<traits::NumDims<T_Src>::value; i++)
                {
                    if(extSrc[i] != extDst[i])
                        throw std::runtime_error("Extents mismatch in dim " + std::to_string(i) + ": "
                                + std::to_string(extSrc[i]) + "!=" + std::to_string(extDst[i]));
                }
            }
        };

    public:
        Copy(){}
        Copy(T_SrcAccessor accSrc, T_DstAccessor accDst): m_accSrc(accSrc), m_accDst(accDst){}

        template< class T_Src, class T_Dst >
        void
        operator()(const T_Src& src, T_Dst& dst)
        {
            using PlainSrc = std::remove_const_t<T_Src>;
            using PlainDst = std::remove_const_t<T_Dst>;
            static constexpr unsigned numDims    = traits::NumDims<PlainSrc>::value;
            static constexpr unsigned numDimsDst = traits::NumDims<PlainDst>::value;
            static_assert(numDims == numDimsDst, "Dimensions must match");
            static constexpr bool srcIsStream = traits::IsStreamAccessor<T_SrcAccessor, PlainSrc>::value;
            static constexpr bool dstIsStream = traits::IsStreamAccessor<T_DstAccessor, PlainDst>::value;
            ExtentsCheck<!srcIsStream && !dstIsStream>::check(src, dst);

            static_assert(DelimiterDimOk<T_SrcAccessor, numDims, srcIsStream>::value,
                    "Source accessor does not provide enough delimiters");
            static_assert(DelimiterDimOk<T_DstAccessor, numDimsDst, dstIsStream>::value,
                    "Destination accessor does not provide enough delimiters");

            loop(src, detail::CopyHandler(), m_accSrc, dst, m_accDst);
        }
    };

    template< class T_SrcAccessor, class T_DstAccessor >
    Copy< std::decay_t<T_SrcAccessor>, std::decay_t<T_DstAccessor> >
    makeCopy(T_SrcAccessor&& accSrc, T_DstAccessor&& accDst){
        return Copy<
                std::decay_t<T_SrcAccessor>,
                std::decay_t<T_DstAccessor>
                >(std::forward<T_SrcAccessor>(accSrc), std::forward<T_DstAccessor>(accDst));
    }

    template<
        class T_Src,
        class T_Dst,
        class T_SrcAccessor = const LiFFT::traits::IdentityAccessor_t<T_Src>,
        class T_DstAccessor = const LiFFT::traits::IdentityAccessor_t<T_Dst>
    >
    void
    copy(const T_Src& src, T_Dst& dst, T_SrcAccessor&& accSrc = T_SrcAccessor(), T_DstAccessor&& accDst = T_DstAccessor())
    {
        makeCopy(std::forward<T_SrcAccessor>(accSrc), std::forward<T_DstAccessor>(accDst))(src, dst);
    }

}  // namespace policies
}  // namespace LiFFT
