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
 
#include "testUtils.hpp"
#include "liFFT/types/Complex.hpp"
#include "liFFT/types/Real.hpp"
#include "liFFT/mem/PlainPtrWrapper.hpp"
#include "liFFT/FFT.hpp"
#include "liFFT/policies/Copy.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace LiFFTTest {

    BOOST_AUTO_TEST_SUITE(PlainPtr)

    BOOST_AUTO_TEST_CASE(PlainPtr)
    {
        static_assert(testNumDims == 2, "Currently only 2D is tested");
        using Real = LiFFT::types::Real<TestPrecision>;
        using Complex = LiFFT::types::Complex<TestPrecision>;
        std::unique_ptr<Real[]> input(new Real[testSize*testSize]);
        std::unique_ptr<Complex[]> output(new Complex[testSize*(testSize/2+1)]);
        for(unsigned i=0; i<testSize*testSize; ++i)
            input[i] = std::rand() / RAND_MAX;
        using FFT_TYPE = LiFFT::FFT_2D_R2C<TestPrecision>;
        auto inWrapped = FFT_TYPE::wrapInput( LiFFT::mem::wrapPtr<false>(input.get(), TestExtents(testSize, testSize)) );
        auto outWrapped = FFT_TYPE::wrapOutput(LiFFT::mem::wrapPtr<true>(output.get(), TestExtents(testSize, testSize/2+1)));
        auto fft = LiFFT::makeFFT<TestLibrary>(inWrapped, outWrapped);
        fft(inWrapped, outWrapped);
        LiFFT::policies::copy(inWrapped, baseR2CInput);
        execBaseR2C();
        checkResult(baseR2COutput, outWrapped, "R2C with PlainPtrWrapper");
    }

    BOOST_AUTO_TEST_SUITE_END()

}  // namespace LiFFTTest
