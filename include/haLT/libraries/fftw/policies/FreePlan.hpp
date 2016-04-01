/* This file is part of HaLT.
 *
 * HaLT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * HaLT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HaLT.  If not, see <www.gnu.org/licenses/>.
 */
 
#pragma once

#include "haLT/libraries/fftw/fftw3Include.h"
#include "haLT/libraries/fftw/traits/LibTypes.hpp"

namespace haLT {
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
}  // namespace haLT