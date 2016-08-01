/* This file is part of libLiFFT.
 *
 * libLiFFT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * libLiFFT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libLiFFT.  If not, see <www.gnu.org/licenses/>.
 */
 
#pragma once

#include <boost/utility.hpp>
#include "liFFT/libraries/fftw/policies/Planner.hpp"
#include "liFFT/libraries/fftw/policies/ExecutePlan.hpp"
#include "liFFT/libraries/fftw/policies/FreePlan.hpp"
#include <boost/mpl/placeholders.hpp>

namespace bmpl = boost::mpl;

namespace LiFFT {
namespace libraries {
namespace fftw {

    /**
     * Wrapper for the CUDA-Library that executes the FFT on GPU(s)
     *
     * \tparam T_FFT_Properties Placeholder that will be replaced by a class containing the properties for this FFT
     */
    template< class T_FFT_Properties = bmpl::_1 >
    class FFTW: private boost::noncopyable
    {
    private:
        using FFT = T_FFT_Properties;
        using Input = typename FFT::Input;
        using Output = typename FFT::Output;
        using PrecisionType = typename FFT::PrecisionType;
        using Planner =
                policies::Planner<
                    PrecisionType,
                    LiFFT::types::TypePair< Input, Output >,
                    FFT::isFwd,
                    FFT::isInplace,
                    FFT::numDims,
                    FFT::isComplexIn,
                    FFT::isComplexOut,
                    FFT::isAoSIn,
                    FFT::isAoSOut,
                    FFT::isStridedIn,
                    FFT::isStridedOut
                >;
        using Executer = policies::ExecutePlan< PrecisionType >;
        using PlanDestroyer = policies::FreePlan< PrecisionType >;
        using PlanType = typename traits::LibTypes< PrecisionType >::PlanType;
        using InPtr = std::result_of_t< decltype(&Input::getDataPtr)(Input) >;
        using OutPtr = std::result_of_t< decltype(&Output::getDataPtr)(Output) >;

        static constexpr unsigned flags = FFT::constructWithReadOnly ? FFTW_ESTIMATE : FFTW_MEASURE;

        PlanType m_plan;
        InPtr m_inPtr;
        OutPtr m_outPtr;

    public:
        FFTW(Input& input, Output& output)
        {
            m_plan = Planner()(input, output, flags);
            m_inPtr = input.getDataPtr();
            m_outPtr = output.getDataPtr();
        }

        explicit FFTW(Input& inOut)
        {
            m_plan = Planner()(inOut, flags);
            m_inPtr = inOut.getDataPtr();
            m_outPtr = nullptr;
        }

        FFTW(FFTW&& obj): m_plan(obj.m_plan), m_inPtr(obj.m_inPtr), m_outPtr(obj.m_outPtr){
            obj.m_plan = nullptr;
        }

        ~FFTW()
        {
            PlanDestroyer()(m_plan);
        }

        void operator()(Input& input, Output& output)
        {
            if(input.getDataPtr() != m_inPtr || output.getDataPtr() != m_outPtr)
                throw std::runtime_error("Pointers to data must not be changed after initialization");

            Executer()(m_plan);
        }

        void operator()(Input& inOut)
        {
            if(inOut.getDataPtr() != m_inPtr)
                throw std::runtime_error("Pointer to data must not be changed after initialization");
            Executer()(m_plan);
        }
    };

}  // namespace fftw
}  // namespace libraries
}  // namespace LiFFT
