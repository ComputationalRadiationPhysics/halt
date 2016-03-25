#pragma once

#include <cassert>
#include <memory>
#include "foobar/c++14_types.hpp"

namespace foobar{
namespace types{

    /**
     * Simple stack-based vector that can be used e.g. for extents, strides...
     */
    template< unsigned T_numDims, typename T = unsigned >
    class Vec
    {
    public:
        static constexpr unsigned numDims = T_numDims;
        using type = T;
        using Ptr = T*;
        using ConstPtr = const T*;
        using Ref = T&;
        using ConstRef = const T&;
        using Storage = std::array< T, numDims >;
        using Iterator = typename Storage::iterator;
        using ConstIterator = typename Storage::const_iterator;

        template<
           typename... TArgs,
           typename = std::enable_if_t<(sizeof...(TArgs) == (numDims-1))>
           >
            Vec(T val, TArgs ... values): values_{std::forward<T>(val), std::forward<TArgs>(values)...}
        {}

        Vec(const Storage& values): values_(values)
        {}

        Vec(){}

        /**
         * Convert to integral value if it is one
         */
        template<typename U, typename = std::enable_if_t< std::is_same<U, T>::value && numDims==1> >
        operator U() const {
            return values_[0];
        }

        static Vec< numDims, type >
        all(const type& val)
        {
            Vec< numDims, type > res;
            for(unsigned i=0; i<numDims; i++)
                res[i] = val;
            return res;
        }

        Ref
        operator[](unsigned dim)
        {
            assert(dim<numDims);
            return values_[dim];
        }

        ConstRef
        operator[](unsigned dim) const
        {
            assert(dim<numDims);
            return values_[dim];
        }

        Ptr
        data()
        {
            return values_.data();
        }

        ConstPtr
        data() const
        {
            return values_.data();
        }

        Iterator
        begin()
        {
            return values_.begin();
        }

        Iterator
        end()
        {
            return values_.end();
        }

        ConstIterator
        cbegin() const
        {
            return values_.cbegin();
        }

        ConstIterator
        cend() const
        {
            return values_.cend();
        }

    private:
        Storage values_;
    };

    template< typename T, T T_val, unsigned T_numDims >
    struct ConstVec
    {
        static constexpr unsigned numDims = T_numDims;
        using type = T;

        constexpr T
        operator[](unsigned dim) const
        {
            return (dim<numDims) ? T_val : throw std::logic_error("Out of range");
        }
    };

    /**
     * Index type for 1D
     */
    using Idx1D = Vec<1, size_t>;
    using Vec1 = Vec<1>;
    /**
     * Index type for 2D
     */
    using Idx2D = Vec<2, size_t>;
    using Vec2 = Vec<2>;
    /**
     * Index type for 3D
     */
    using Idx3D = Vec<3, size_t>;
    using Vec3 = Vec<3>;

}  // namespace types
}  // namespace foobar
