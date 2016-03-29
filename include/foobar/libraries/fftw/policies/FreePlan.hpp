#pragma once

#include "foobar/libraries/fftw/fftw3Include.h"
#include "foobar/libraries/fftw/traits/LibTypes.hpp"

namespace foobar {
namespace libraries {
namespace fftw {
namespace policies{

    /**
     * Frees a given plan
     */
    template< typename T_Precision >
    struct FreePlan;

    template<>
    struct FreePlan<float>
    {
        using PlanType = typename traits::LibTypes<float>::PlanType;

        void
        operator()(PlanType& plan)
        {
            fftwf_destroy_plan(plan);
        }
    };

    template<>
    struct FreePlan<double>
    {
        using PlanType = typename traits::LibTypes<double>::PlanType;

        void
        operator()(PlanType& plan)
        {
            fftw_destroy_plan(plan);
        }
    };

}  // namespace policies
}  // namespace fftw
}  // namespace libraries
}  // namespace foobar
