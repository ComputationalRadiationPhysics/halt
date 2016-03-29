#pragma once

#include "foobar/traits/IsBinaryCompatible.hpp"

namespace foobar {
namespace types {

    template< typename T >
    struct Real
    {
        using type = T;
        static constexpr bool isComplex = false;
        T value;

        Real(){}
        Real(T val):value(val){}
        operator T() const {return value;}
    };

}  // namespace types

namespace traits {

    template< typename T >
    struct IsBinaryCompatibleImpl< types::Real<T>, T >: std::true_type{};

}  // namespace traits
}  // namespace foobar
