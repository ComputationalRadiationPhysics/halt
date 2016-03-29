#pragma once

#include <memory>
#include "foobar/traits/IntegralType.hpp"
#include "foobar/traits/IsComplex.hpp"
#include "foobar/accessors/ArrayAccessor.hpp"

#include <cassert>

namespace foobar {
namespace mem {
namespace detail {

    /**
     * Deleter that does nothing
     */
    struct NopDeleter
    {
        template< typename T >
        void
        operator()(T){}
    };

    /**
     * Wrapper to hold and manage an Array of Structs
     *
     * \tparam T Type to hold
     * \tparam T_ownsPointer Whether this class owns its pointer or not (memory is freed on destroy, when true)
     */
    template< typename T, bool T_ownsPointer = true >
    class AoSValues
    {
    public:
        using type = typename traits::IntegralType<T>::type;
        static constexpr bool ownsPointer = T_ownsPointer;
        static constexpr bool isComplex = traits::IsComplex<T>::value;
        static constexpr bool isAoS = true;
        using Value = T;
        using Ptr = Value*;
        using Ref = Value&;
        using ConstRef = const Value&;
        using Data = std::conditional_t<
                        ownsPointer,
                        std::unique_ptr< Value[] >,
                        std::unique_ptr< Value[], NopDeleter >
                     >;
        using IdentityAccessor = accessors::ArrayAccessor<>;

        AoSValues(): AoSValues(nullptr, 0){}
        AoSValues(Ptr data, size_t numElements): data_(data), numElements_(numElements){}

        void
        reset(Ptr data, size_t numElements)
        {
            assert(numElements || !data);
            data_.reset(data);
            numElements_ = numElements;
        }

        void
        allocData(size_t numElements)
        {
            assert(numElements);
            data_.reset(new Value[numElements]);
            numElements_ = numElements;
        }

        void
        freeData()
        {
            data_.reset();
            numElements_ = 0;
        }

        Ptr
        releaseData()
        {
            numElements_ = 0;
            return data_.release();
        }

        Ptr
        getData() const
        {
            return data_.get();
        }

        size_t
        getNumElements() const
        {
            return numElements_;
        }

        size_t
        getMemSize() const
        {
            return numElements_ * sizeof(T);
        }

        ConstRef
        operator[](size_t idx) const
        {
            assert(idx < numElements_);
            return data_[idx];
        }

        Ref
        operator[](size_t idx)
        {
            assert(idx < numElements_);
            return data_[idx];
        }

    private:
        Data data_;
        size_t numElements_;
    };

}  // namespace detail
}  // namespace mem
}  // namespace foobar
