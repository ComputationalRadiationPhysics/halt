#pragma once

#include "foobar/types/Real.hpp"
#include "foobar/mem/AoSValues.hpp"

namespace foobar {
namespace mem {

    template< typename T, bool T_ownsPointer = true >
    class RealValues: public detail::AoSValues< types::Real<T>, T_ownsPointer >
    {
    public:
        using Parent = detail::AoSValues< types::Real<T>, T_ownsPointer >;

        using Parent::Parent;
    };

}  // namespace mem
}  // namespace foobar
