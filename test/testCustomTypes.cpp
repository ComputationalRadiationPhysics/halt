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

#include "libLiFFT/traits/IsComplex.hpp"
#include "libLiFFT/traits/IsBinaryCompatible.hpp"
#include "libLiFFT/traits/IntegralType.hpp"
#include "libLiFFT/FFT.hpp"
#include "Volume.hpp"
#include "libLiFFT/accessors/VolumeAccessor.hpp"
#if !defined(WITH_CUDA) && !defined(WITH_OPENCL)
#   include "libLiFFT/libraries/fftw/fftw3Include.h"
#endif
#include "libLiFFT/generateData.hpp"
#include "libLiFFT/policies/Copy.hpp"
#include "libLiFFT/traits/IdentityAccessor.hpp"
#include "libLiFFT/types/SliceView.hpp"
#include <boost/test/unit_test.hpp>
#include <type_traits>
#include <iostream>

using LiFFT::generateData;
using namespace LiFFT::generators;

namespace LiFFTTest {

    template< typename T = double >
    struct MyComplex{
        T real, imag;
        MyComplex(){}
        MyComplex(T realIn): real(realIn), imag(0){}
        MyComplex(T realIn, T imagIn): real(realIn), imag(imagIn){}
    };

}  // namespace LiFFTTest

namespace LiFFT {
    namespace traits {

        template<typename T>
        struct IsComplex< LiFFTTest::MyComplex<T> >: std::true_type{};

        template<typename T>
        struct IsBinaryCompatibleImpl< LiFFTTest::MyComplex<T>, LiFFT::types::Complex<T> >: std::true_type{};

        template<typename T>
        struct IntegralType< LiFFTTest::MyComplex<T> >
        {
            using type = T; // or define this in MyComplex itself
        };

#if !defined(WITH_CUDA) && !defined(WITH_OPENCL)
        template<>
        struct IsComplex< fftw_complex >: std::true_type{};

        template<>
        struct IsComplex< fftwf_complex >: std::true_type{};

        template<>
        struct IsBinaryCompatibleImpl< fftw_complex, LiFFT::types::Complex<double> >: std::true_type{};
        template<>
        struct IsBinaryCompatibleImpl< fftwf_complex, LiFFT::types::Complex<float> >: std::true_type{};
#endif

        template<typename T>
        struct IdentityAccessor< Volume<T> >
        {
            using type = accessors::VolumeAccessor;
        };
    }  // namespace traits

}  // namespace LiFFT

namespace LiFFTTest {

    template<typename T>
    std::ostream& operator<< (std::ostream& stream, MyComplex<T> val){
        stream << val.real << " " << val.imag;
        return stream;
    }

    using ComplexVol     = Volume< MyComplex<TestPrecision> >;
#if !defined(WITH_CUDA) && !defined(WITH_OPENCL)
    using ComplexVolFFTW = Volume< std::conditional_t<std::is_same<TestPrecision, float>::value, fftwf_complex, fftw_complex > >;
#else
    using ComplexVolFFTW = ComplexVol;
#endif
    using RealVol        = Volume< TestPrecision >;

    using LiFFT::types::makeRange;
    using LiFFT::types::makeSliceView;

    BOOST_AUTO_TEST_SUITE(CustomTypes)

    BOOST_AUTO_TEST_CASE(Complex)
    {
        using LiFFT::accessors::VolumeAccessor;
        auto aperture = ComplexVol(testSize, testSize);
        auto fftResult = ComplexVol(aperture.xDim(), aperture.yDim(), aperture.zDim());
        using FFT_Type = LiFFT::FFT_2D_C2C<TestPrecision>;
        auto input = FFT_Type::wrapInput(makeSliceView<0>(aperture));
        auto output = FFT_Type::wrapOutput(makeSliceView<0>(fftResult));
        auto fft = LiFFT::makeFFT<TestLibrary>(input, output);
        generateData(aperture, Rect<TestPrecision>(20,testSize/2));
        fft(input, output);
        LiFFT::policies::copy(makeSliceView<0>(aperture), baseC2CInput);
        execBaseC2C();
        checkResult(baseC2COutput, makeSliceView<0>(fftResult), "C2C with custom types");
    }

    BOOST_AUTO_TEST_CASE(Real)
    {
        using LiFFT::accessors::VolumeAccessor;
        auto aperture = RealVol(testSize, testSize);
        auto fftResult = ComplexVolFFTW(aperture.xDim()/2+1, aperture.yDim(), aperture.zDim());
        using FFT_Type = LiFFT::FFT_2D_R2C<TestPrecision>;
        auto input = FFT_Type::wrapInput(makeSliceView<0>(aperture));
        auto output = FFT_Type::wrapOutput(makeSliceView<0>(fftResult));
        auto fft = LiFFT::makeFFT<TestLibrary>(input, output);
        generateData(aperture, Rect<TestPrecision>(20,testSize/2));
        fft(input, output);
        LiFFT::policies::copy(makeSliceView<0>(aperture), baseR2CInput);
        execBaseR2C();
        checkResult(baseR2COutput, makeSliceView<0>(fftResult), "R2C with custom types");
    }

    BOOST_AUTO_TEST_SUITE_END()

}  // namespace LiFFTTest
