#pragma once

#include "foobar/policies/GetExtents.hpp"
#include "foobar/policies/GetStrides.hpp"
#include "foobar/traits/NumDims.hpp"
#include "foobar/traits/IsStrided.hpp"
#include "foobar/c++14_types.hpp"
#include <cassert>

namespace foobar {
namespace policies {

    /**
     * Makes an index "flat", that is: vector indices are converted to an unsigned
     */
    template< class T_Data, bool T_IsStrided = traits::IsStrided<T_Data>::value >
    struct FlattenIdx
    {
        template< class T_Index >
        std::enable_if_t< std::is_integral<std::remove_reference_t<T_Index>>::value, size_t >
        operator()(T_Index&& idx, const T_Data& /*data*/) const
        {
            return idx;
        }

        template< class T_Index >
        std::enable_if_t< !std::is_integral<std::remove_reference_t<T_Index>>::value, size_t >
        operator()(T_Index&& idx, const T_Data& data) const
        {
            static constexpr unsigned numDims = traits::NumDims<T_Data>::value;
            GetExtents<T_Data> extents(data);
            assert(checkSizes(idx, extents));
            size_t flatIdx = idx[0];
            for(unsigned i=1; i<numDims; ++i)
                flatIdx = flatIdx*extents[i] + idx[i];
            return flatIdx;
        }
    };

    template< class T_Data >
    struct FlattenIdx< T_Data, true>
    {
        template< class T_Index >
        std::enable_if_t< std::is_integral<T_Index>::value, size_t >
        operator()(T_Index&& idx, const T_Data& /*data*/) const
        {
            return idx;
        }

        template< class T_Index >
        std::enable_if_t< !std::is_integral<T_Index>::value, size_t >
        operator()(T_Index&& idx, const T_Data& data) const
        {
            static constexpr unsigned numDims = traits::NumDims<T_Data>::value;
#ifndef NDEBUG
            GetExtents<T_Data> extents(data);
            assert(checkSizes(idx, extents));
#endif
            GetStrides<T_Data> strides(data);
            size_t flatIdx = 0;
            for(unsigned i=0; i<numDims; ++i)
                flatIdx += idx[i] * strides[i];
            return flatIdx;
        }
    };

    template< class T_Index, class T_Data >
    size_t
    flattenIdx(T_Index&& idx, const T_Data& data)
    {
        return FlattenIdx<T_Data>()(std::forward<T_Index>(idx), data);
    }

}  // namespace policies
}  // namespace foobar
