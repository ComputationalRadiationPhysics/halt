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
#include "liFFT/FFT.hpp"
#include "liFFT/generateData.hpp"
#include "liFFT/types/View.hpp"
#include "liFFT/types/SliceView.hpp"
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <cmath>

using LiFFT::generateData;
using namespace LiFFT::generators;

namespace LiFFTTest {

    BOOST_AUTO_TEST_SUITE(Inplace)

    struct GenData
    {

        template< class T_Idx >
        float operator()(T_Idx&& idx) const
        {
            return idx[1] + idx[0]*1024;
        }
    };

    BOOST_AUTO_TEST_CASE(ContainerCreation)
    {
        using FFT = LiFFT::FFT_3D_R2C_F<true>;
        auto input = FFT::createNewInput(LiFFT::types::Vec3(2048u, 1024u, 1u));
        LiFFT::mem::RealContainer<2, float> data(LiFFT::types::Vec2(2048u, 1024u));
        LiFFT::mem::RealContainer<3, float> data2(LiFFT::types::Vec3(2048u, 1024u, 1u));
        auto view = LiFFT::types::makeSliceView<2>(input, LiFFT::types::makeRange());
        LiFFT::generateData(view, GenData());
        LiFFT::policies::copy(view, data);
        LiFFT::policies::copy(input, data2);
        for(unsigned z = 0; z < input.getExtents()[2]; z++)
            for(unsigned y = 0; y < input.getExtents()[1]; y++)
            {
                float expected = y + z * input.getExtents()[1];
                float is = data(LiFFT::types::Vec2(z, y));
                float diff = std::abs(expected - is);
                BOOST_REQUIRE_SMALL(diff, 1e-6f);
                is = data2(LiFFT::types::Vec3(z, y, 0u));
                diff = std::abs(expected - is);
                BOOST_REQUIRE_SMALL(diff, 1e-6f);
            }
    }

    BOOST_AUTO_TEST_CASE(InplaceComplex)
    {
        auto aperture = ComplexContainer(TestExtents::all(testSize));
        using FFT_Type = LiFFT::FFT_2D_C2C<TestPrecision, true>;
        auto input = FFT_Type::wrapInput(aperture);
        auto output = FFT_Type::createNewOutput(input);
        auto fft = LiFFT::makeFFT<TestLibrary>(input);
        generateData(input, Rect<TestPrecision>(20,testSize/2));
        LiFFT::policies::copy(aperture, baseC2CInput);
        fft(input);
        execBaseC2C();
        // Inplace got some more random derivations in the low intensity regions
        checkResult(baseC2COutput, output, "C2C inPlace", CmpError(1e-3, 5e-5));
    }

    BOOST_AUTO_TEST_CASE(InplaceR2C)
    {
        TestExtents ext = TestExtents::all(testSize);
        ext[testNumDims - 1] = (ext[testNumDims - 1] / 2 + 1) * 2;
        using namespace LiFFT::types;
        auto aperture = makeView(RealContainer(ext), makeRange(Origin(), TestExtents::all(testSize)));
        using FFT_Type = LiFFT::FFT_2D_R2C<TestPrecision, true>;
        auto input = FFT_Type::wrapInput(aperture);
        auto output = FFT_Type::createNewOutput(input);
        auto fft = LiFFT::makeFFT<TestLibrary>(input);
        generateData(input, Rect<TestPrecision>(20,testSize/2));
        LiFFT::policies::copy(aperture, baseR2CInput);
        fft(input);
        execBaseR2C();
        // Inplace got some more random derivations in the low intensity regions
        checkResult(baseR2COutput, output, "R2C inPlace", CmpError(1e-3, 5e-5));
    }

    BOOST_AUTO_TEST_SUITE_END()
}  // namespace LiFFTTest
