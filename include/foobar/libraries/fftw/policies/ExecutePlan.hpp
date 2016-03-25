#pragma once

#include "foobar/libraries/fftw/fftw3Include.h"
#include "foobar/libraries/fftw/traits/LibTypes.hpp"

namespace foobar {
namespace libraries {
namespace fftw {
namespace policies{

    /**
     * Executes a given plan
     */
    template< typename T_Precision >
    struct ExecutePlan;

    template<>
    struct ExecutePlan<float>
    {
        using PlanType = typename traits::LibTypes<float>::PlanType;

        void
        operator()(PlanType& plan)
        {
            fftwf_execute(plan);
        }
    };

    template<>
    struct ExecutePlan<double>
    {
        using PlanType = typename traits::LibTypes<double>::PlanType;

        void
        operator()(PlanType& plan)
        {
            fftw_execute(plan);
        }
    };

}  // namespace policies
}  // namespace fftw
}  // namespace libraries
}  // namespace foobar
