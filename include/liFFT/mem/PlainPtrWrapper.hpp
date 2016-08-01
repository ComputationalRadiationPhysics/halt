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
#include "liFFT/mem/DataContainer.hpp"
#include "liFFT/types/Vec.hpp"
#include "liFFT/types/Real.hpp"
#include "liFFT/types/Complex.hpp"
#include "liFFT/util.hpp"
#include "liFFT/policies/SafePtrCast.hpp"
#include "liFFT/accessors/ArrayAccessor.hpp"
#include "liFFT/c++14_types.hpp"
#include "liFFT/traits/IntegralType.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/traits/IsDeviceMemory.hpp"

namespace LiFFT {
namespace mem {

    /**
     * Wrapper to use plain pointers in the framework
     * This adds dimensions, extents and strides
     */
    template< class T_NumDims, typename T_Type, class T_IsStrided, class T_IsDevicePtr >
    class PlainPtrWrapper: protected DataContainer< T_NumDims::value, T_Type*, void, T_IsStrided::value>
    {
    protected:
        using Parent = DataContainer< T_NumDims::value, T_Type*, void, T_IsStrided::value>;
    public:
        static constexpr unsigned numDims = T_NumDims::value;
        using Type = T_Type;
        static constexpr bool isStrided = T_IsStrided::value;
        static constexpr bool isDevicePtr = T_IsDevicePtr::value;
        using IntegralType = typename traits::IntegralType<Type>::type;
        static constexpr bool isComplex = traits::IsComplex<Type>::value;

        static_assert(std::is_same< Type, types::Real<IntegralType> >::value ||
                std::is_same< Type, types::Complex<IntegralType> >::value,
                "You must use build-in types!");

        using IdxType = typename Parent::IdxType;
        using IdentityAccessor = accessors::ArrayAccessor< true >;

        using Pointer = Type*;
        using Ref = Type&;
        using ConstRef = const Type&;

        friend struct policies::GetExtents<PlainPtrWrapper>;

        template<typename T_Extents>
        PlainPtrWrapper(Pointer ptr, T_Extents&& extents): Parent(ptr, extents)
        {
            static_assert(!isStrided, "You need to specify the strides!");
        }

        template<typename T_Extents>
        PlainPtrWrapper(IntegralType* ptr, T_Extents&& extents):
            PlainPtrWrapper(policies::safe_ptr_cast<Pointer>(ptr), extents)
        {}

        template<typename T_Extents, typename T_Strides>
        PlainPtrWrapper(Pointer ptr, T_Extents&& extents, T_Strides&& strides): Parent(ptr, extents, strides)
        {
            static_assert(isStrided, "You cannot specify strides!!");
        }

        template<typename T_Extents, typename T_Strides>
        PlainPtrWrapper(IntegralType* ptr, T_Extents&& extents, T_Strides&& strides):
            PlainPtrWrapper(policies::safe_ptr_cast<Pointer>(ptr), extents, strides)
        {}

        template< class T_Index >
        Ref
        operator()(T_Index&& idx)
        {
            assert(policies::checkSizes(idx, this->getExtents()));
            unsigned flatIdx = policies::flattenIdx(idx, *this);
            return this->m_data[flatIdx];
        }

        template< class T_Index >
        ConstRef
        operator()(T_Index&& idx) const
        {
            assert(policies::checkSizes(idx, this->getExtents()));
            unsigned flatIdx = policies::flattenIdx(idx, *this);
            return this->m_data[flatIdx];
        }

        size_t
        getMemSize() const
        {
            return policies::getNumElements(*this, false) * sizeof(Type);
        }

        using Parent::getExtents;
    };

    /**
     * Evaluates to Real<T> or Complex<T> depending on the template argument
     */
    template< bool T_isComplex, typename T >
    struct RealOrComplex
    {
        using IntegralType = traits::IntegralType_t<T>;
        using type = std::conditional_t< T_isComplex, types::Complex<IntegralType>, types::Real<IntegralType> >;
        static_assert(std::is_floating_point<T>::value || std::is_same<T, type>::value,
                "Only floating point, Real or Complex values are allowed!");
    };
    template< bool T_isComplex, typename T >
    using RealOrComplex_t = typename RealOrComplex< T_isComplex, T >::type;


    // Following are the convenience functions for wrapping pointers

    template< bool T_isComplex, bool T_isDevicePtr = false, typename T = float, unsigned numDims = 1, typename U >
    PlainPtrWrapper< UnsignedConst<numDims>, RealOrComplex_t< T_isComplex, T >, std::false_type, BoolConst<T_isDevicePtr> >
    wrapPtr(T* ptr, const types::Vec<numDims, U>& size)
    {
        return PlainPtrWrapper< UnsignedConst<numDims>, RealOrComplex_t< T_isComplex, T >, std::false_type, BoolConst<T_isDevicePtr> >(ptr, size);
    }

    template< bool T_isComplex, bool T_isDevicePtr = false, typename T = float, unsigned numDims = 1, typename U, typename V >
    PlainPtrWrapper< UnsignedConst<numDims>, RealOrComplex_t< T_isComplex, T >, std::true_type, BoolConst<T_isDevicePtr> >
    wrapPtrStrided(T* ptr, const types::Vec<numDims, U>& size, const types::Vec<numDims, V>& stride)
    {
        return PlainPtrWrapper< UnsignedConst<numDims>, RealOrComplex_t< T_isComplex, T >, std::true_type, BoolConst<T_isDevicePtr> >(ptr, size, stride);
    }

}  // namespace mem

namespace traits {

    template< class... T >
    struct IsDeviceMemory< mem::PlainPtrWrapper<T... > >: std::integral_constant<bool, mem::PlainPtrWrapper<T... >::isDevicePtr>{};

    template< class... T >
    struct IsAoS< mem::PlainPtrWrapper<T... > >: std::integral_constant<bool, true>{};

}  // namespace traits

}  // namespace LiFFT
