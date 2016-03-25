#pragma once

#include "foobar/policies/flattenIdx.hpp"
#include "foobar/mem/DataContainer.hpp"
#include "foobar/types/Vec.hpp"
#include "foobar/types/Real.hpp"
#include "foobar/types/Complex.hpp"
#include "foobar/util.hpp"
#include "foobar/policies/SafePtrCast.hpp"
#include "foobar/accessors/ArrayAccessor.hpp"
#include "foobar/c++14_types.hpp"
#include "foobar/traits/IntegralType.hpp"
#include "foobar/traits/IsComplex.hpp"
#include "foobar/traits/IsDeviceMemory.hpp"

namespace foobar {
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
        static constexpr bool isAoS = true;

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
        PlainPtrWrapper(Pointer ptr, T_Extents&& rExtents): Parent(ptr, rExtents)
        {
            static_assert(!isStrided, "You need to specify the strides!");
        }

        template<typename T_Extents>
        PlainPtrWrapper(IntegralType* ptr, T_Extents&& rExtents):
            PlainPtrWrapper(policies::safe_ptr_cast<Pointer>(ptr), rExtents)
        {}

        template<typename T_Extents, typename T_Strides>
        PlainPtrWrapper(Pointer ptr, T_Extents&& rExtents, T_Strides&& strides): Parent(ptr, rExtents, strides)
        {
            static_assert(isStrided, "You cannot specify strides!!");
        }

        template<typename T_Extents, typename T_Strides>
        PlainPtrWrapper(IntegralType* ptr, T_Extents&& rExtents, T_Strides&& strides):
            PlainPtrWrapper(policies::safe_ptr_cast<Pointer>(ptr), rExtents, strides)
        {}

        template< class T_Index >
        Ref
        operator()(T_Index&& idx)
        {
            assert(policies::checkSizes(idx, this->getExtents()));
            unsigned flatIdx = policies::flattenIdx(idx, *this);
            return this->data[flatIdx];
        }

        template< class T_Index >
        ConstRef
        operator()(T_Index&& idx) const
        {
            assert(policies::checkSizes(idx, this->getExtents()));
            unsigned flatIdx = policies::flattenIdx(idx, *this);
            return this->data[flatIdx];
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

}  // namespace traits

}  // namespace foobar
