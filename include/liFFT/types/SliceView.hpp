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

#include "liFFT/types/Vec.hpp"
#include "liFFT/traits/IdentityAccessor.hpp"
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/accessors/ArrayAccessor.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/types/Range.hpp"
#include "liFFT/traits/IntegralType.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/traits/IsStrided.hpp"
#include "liFFT/traits/IsAoS.hpp"

namespace LiFFT {
namespace types {

    /**
     * Provides a view on a container
     * Outside users will see it like it has a specific extent which is only a part of the real extents
     * The view is specified by an offset (in each dimension) and the extents (in each dimension)
     */
    template<
        class T_Base,
        unsigned T_fixedDim,
        typename T_HasInstance,
        class T_BaseAccessor = traits::IdentityAccessor_t<T_Base>
    >
    class SliceView
    {
        using Base = T_Base;
        static constexpr unsigned fixedDim = T_fixedDim;
        static constexpr bool hasInstance = T_HasInstance::value;
        using BaseAccessor = T_BaseAccessor;
        static constexpr unsigned baseNumDims = traits::NumDims<Base>::value;
        static_assert(baseNumDims > 1, "Cannot remove last dimension");
        static_assert(fixedDim < baseNumDims, "Fixed dimension does not exist");

        using RefType = typename std::add_lvalue_reference<Base>::type;
        using InstanceType = std::conditional_t< hasInstance, Base, RefType >;
        using ParamType = typename std::conditional_t< hasInstance, std::add_rvalue_reference<Base>, std::add_lvalue_reference<Base> >::type;
    public:
        static constexpr unsigned numDims = baseNumDims - 1;
        using Extents = Vec<numDims>;
        using BaseExtents = Vec<baseNumDims>;

    private:
        InstanceType m_base;
        BaseAccessor m_acc;
        BaseExtents m_offsets;
        Extents m_extents;

        constexpr unsigned getIdx(unsigned baseIdx) const
        {
            return (baseIdx>fixedDim) ? baseIdx-1 : baseIdx;
        }

        Extents
        removeDimFromVec(const BaseExtents& vec)
        {
            Extents res;
            for(unsigned i=0; i<baseNumDims; ++i)
            {
                if(i != fixedDim)
                    res[getIdx(i)] = vec[i];
            }
            return res;
        }
    public:
        using IdentityAccessor = accessors::ArrayAccessor<true>;

        /**
         * Creates a view with offsets and extents
         * Validation on those is performed at runtime
         *
         * @param base Base container
         * @param offsets Offsets for each dimension
         * @param extents New extents
         * @param acc Accessor to access the base class
         */
        SliceView(ParamType base, const BaseExtents& offsets, const Extents& extents, const BaseAccessor& acc = BaseAccessor()):
            m_base(static_cast<ParamType>(base)), m_acc(acc), m_offsets(offsets), m_extents(extents)
        {
            policies::GetExtents<Base> bExtents(m_base);
            for(unsigned i=0; i<baseNumDims; ++i)
            {
                if(offsets[i] >= bExtents[i])
                    throw std::runtime_error("Invalid offset");
                if(i != fixedDim)
                {
                    if(offsets[i] + extents[getIdx(i)] > bExtents[i])
                        throw std::runtime_error("Invalid extents");
                }
            }
        }

        /**
         * Creates a view with offsets and extents
         * Validation on those is performed at runtime
         * This accepts a full extents vector (from base type) and deletes the fixed dimension (implicitly setting it to 1)
         *
         * @param base Base container
         * @param offsets Offsets for each dimension
         * @param extents New extents
         * @param acc Accessor to access the base class
         */
        SliceView(ParamType base, const BaseExtents& offsets, const BaseExtents& extents, const BaseAccessor& acc = BaseAccessor()):
            SliceView(static_cast<ParamType>(base), offsets, removeDimFromVec(extents), acc)
        {}

        template<typename T_Idx>
        std::result_of_t< BaseAccessor(const BaseExtents&, Base&) >
        operator()(const T_Idx& idx)
        {
            static_assert(traits::NumDims<T_Idx>::value == numDims, "Wrong Idx dimensions");
            BaseExtents idxNew;
            for(unsigned i=0; i<baseNumDims; ++i)
                idxNew[i] = (i==fixedDim) ? m_offsets[i] : m_offsets[i] + idx[getIdx(i)];
            return m_acc(idxNew, m_base);
        }

        template<typename T_Idx>
        std::result_of_t< BaseAccessor(const BaseExtents&, const Base&) >
        operator()(const T_Idx& idx) const
        {
            static_assert(traits::NumDims<T_Idx>::value == numDims, "Wrong Idx dimensions");
            BaseExtents idxNew;
            for(unsigned i=0; i<baseNumDims; ++i)
                idxNew[i] = (i==fixedDim) ? m_offsets[i] : m_offsets[i] + idx[getIdx(i)];
            const Base& cBase = const_cast<const Base&>(m_base);
            return m_acc(idxNew, cBase);
        }

        /**
         * Returns a reference to the base class
         * @return Reference to base data
         */
        RefType
        getBase()
        {
            return m_base;
        }

        const Extents&
        getExtents() const
        {
            return m_extents;
        }

        size_t
        getMemSize() const
        {
            return traits::getMemSize(m_base);
        }
    };

    template<
            unsigned T_fixedDim,
            class T_Base,
            class T_BaseAccessor = traits::IdentityAccessor_t<std::remove_reference_t<T_Base>>,
            class T_Range = Range<>
        >
    SliceView< std::remove_reference_t<T_Base>, T_fixedDim, negate< std::is_lvalue_reference<T_Base> >, T_BaseAccessor >
    makeSliceView(T_Base&& base, const T_Range& range = T_Range(), const T_BaseAccessor& acc = T_BaseAccessor())
    {
        using Base = std::remove_cv_t<std::remove_reference_t<T_Base>>;
        return SliceView< std::remove_reference_t<T_Base>, T_fixedDim, negate< std::is_lvalue_reference<T_Base> >, T_BaseAccessor >(
                std::forward<T_Base>(base),
                GetRangeOffset<T_Range, Base>::get(range),
                GetRangeExtents<T_Range, Base>::get(range, base),
                acc);
    }

}  // namespace types

namespace traits {

    template< class T_Base, unsigned u, class... T >
    struct IntegralType< types::SliceView< T_Base, u, T... > >: IntegralType<T_Base>{};

    template< class T_Base, unsigned u, class... T >
    struct IsComplex< types::SliceView< T_Base, u, T... > >: IsComplex<T_Base>{};

    template< class T_Base, unsigned u, class... T >
    struct IsStrided< types::SliceView< T_Base, u, T... > >: IsStrided<T_Base>{};

    template< class T_Base, unsigned u, class... T >
    struct IsAoS< types::SliceView< T_Base, u, T... > >: IsAoS<T_Base>{};

}  // namespace traits

}  // namespace LiFFT
