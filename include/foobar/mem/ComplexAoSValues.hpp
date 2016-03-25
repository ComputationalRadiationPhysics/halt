#pragma once

#include <memory>
#include "foobar/types/Complex.hpp"
#include "foobar/mem/AoSValues.hpp"

namespace foobar {
namespace mem {

    template< typename T, bool T_ownsPointer = true >
    class ComplexAoSValues: public detail::AoSValues< types::Complex<T>, T_ownsPointer >
    {
    public:
        using Parent = detail::AoSValues< types::Complex<T>, T_ownsPointer >;

        using Parent::Parent;
    };

}  // namespace mem
}  // namespace foobar
