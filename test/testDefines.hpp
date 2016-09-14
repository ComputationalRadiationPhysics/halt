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

#include "libLiFFT/mem/DataContainer.hpp"
#include "libLiFFT/mem/RealValues.hpp"
#include "libLiFFT/mem/ComplexAoSValues.hpp"
#ifdef WITH_CUDA
#include "libLiFFT/libraries/cuFFT/cuFFT.hpp"
#elif defined(WITH_OPENCL)
#include "libLiFFT/libraries/clFFT/policies/Context.hpp"
#include "libLiFFT/libraries/clFFT/clFFT.hpp"
#else
#include "libLiFFT/libraries/fftw/FFTW.hpp"
#endif

namespace LiFFTTest{
    // Types used for the test suite
    constexpr unsigned testNumDims = 2;
    constexpr unsigned testSize = 2048;
    using TestExtents      = LiFFT::types::Vec<testNumDims>;
    using TestPrecision    = float;
    using RealType         = LiFFT::mem::RealValues<TestPrecision>;
    using ComplexType      = LiFFT::mem::ComplexAoSValues<TestPrecision>;
    using RealContainer    = LiFFT::mem::DataContainer<testNumDims, RealType>;
    using ComplexContainer = LiFFT::mem::DataContainer<testNumDims, ComplexType>;
    using BaseR2CInput  = RealContainer;
    using BaseR2COutput = ComplexContainer;
    using BaseC2CInput  = ComplexContainer;
    using BaseC2COutput = ComplexContainer;

    #ifdef WITH_CUDA
    using TestLibrary = LiFFT::libraries::cuFFT::CuFFT<>;
    #elif defined(WITH_OPENCL)
  /**
   * For test of ContextWrapper following needs to be placed in a function
    before using clfft:
    LiFFT::libraries::clFFT::policies::ContextLocal cl;
    bool create_clfft_context=true;
    LiFFT::libraries::clFFT::policies::ContextWrapper::wrap(
      cl.context(),
      cl.device(),
      [cl.queue(),]
      create_clfft_context);
  */
    //using Context = LiFFT::libraries::clFFT::policies::ContextWrapper; */
    //using Context = LiFFT::libraries::clFFT::policies::ContextLocal;
    using Context = LiFFT::libraries::clFFT::policies::ContextGlobal;
    using TestLibrary = LiFFT::libraries::clFFT::ClFFT<Context>;
    #else
    using TestLibrary = LiFFT::libraries::fftw::FFTW<>;
    #endif

    // Control values used in the test suite
    extern BaseR2CInput  baseR2CInput;
    extern BaseR2COutput baseR2COutput;
    extern BaseC2CInput  baseC2CInput;
    extern BaseC2COutput baseC2COutput;
}
