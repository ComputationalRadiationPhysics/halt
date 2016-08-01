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
 
#include "tiffWriter/image.hpp"
#include "tiffWriter/traitsAndPolicies.hpp"
#include "liFFT/FFT.hpp"
#if defined(WITH_CUDA)
#   include "liFFT/libraries/cuFFT/cuFFT.hpp"
    using FFT_LIB = LiFFT::libraries::cuFFT::CuFFT<>;
#else
#   include "liFFT/libraries/fftw/FFTW.hpp"
    using FFT_LIB = LiFFT::libraries::fftw::FFTW<>;
#endif

#include "liFFT/policies/Copy.hpp"
#include "liFFT/accessors/TransformAccessor.hpp"
#include "liFFT/accessors/TransposeAccessor.hpp"
#include "liFFT/policies/CalcIntensityFunctor.hpp"
#include "liFFT/generateData.hpp"
#include "liFFT/types/SliceView.hpp"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <cmath>

namespace po = boost::program_options;
using std::string;
using std::cout;
using std::cerr;

namespace options {
    po::options_description desc("Generate 3D data, convert with FFT and output to tiff");
}  // namespace options

void showHelp()
{
    cout << options::desc << std::endl;
}

template<typename T>
inline T
gauss(T x, T xOffset, T scale, T delta)
{
    return scale * std::exp(-std::pow(x - xOffset, 2) / (T(2) * std::pow(delta, 2) ));
}

template<typename T>
inline T
raiseFunc(T x, T xOffset, T scale, T delta)
{
    return scale / (T(1) + std::exp(-(x - xOffset) * delta));
}

template<typename T>
inline T
sinFunc(T x, T xOffset, T scale, T freq)
{
    return scale * std::sin((x - xOffset) * freq);
}

template<typename T>
struct Values1_2P5um
{
    static constexpr T raise1Max = 0.65;
    static constexpr T raise1Delta = 0.01;
    static constexpr T raise1Offset = -100;
    static constexpr T raise2Max = 1;
    static constexpr T raise2Delta = 0.02;
    static constexpr T raise2Offset = 100;
    static constexpr T gaussMax = 2.4;
    static constexpr T gaussDelta = 150;
    static constexpr T gaussOffset = -350;
    static constexpr T scale = 0.55;
};

template<typename T>
struct Values1_1um
{
    static constexpr T raise1Max = 0.663;
    static constexpr T raise1Delta = 0.01;
    static constexpr T raise1Offset = -100;
    static constexpr T raise2Max = 1;
    static constexpr T raise2Delta = 0.046;
    static constexpr T raise2Offset = 40;
    static constexpr T gaussMax = 2.37;
    static constexpr T gaussDelta = 150;
    static constexpr T gaussOffset = -350;
    static constexpr T scale = 0.55;
};

template<typename T>
struct Values1_100nm
{
    static constexpr T raise1Max = 0.6875;
    static constexpr T raise1Delta = 0.01;
    static constexpr T raise1Offset = -100;
    static constexpr T raise2Max = 1;
    static constexpr T raise2Delta = 0.395;
    static constexpr T raise2Offset = 4;
    static constexpr T gaussMax = 2.324;
    static constexpr T gaussDelta = 150;
    static constexpr T gaussOffset = -350;
    static constexpr T scale = 0.55;
};

template<typename T>
struct Values2
{
    static constexpr T raise1Max = 0.65;
    static constexpr T raise1Delta = 0.007;
    static constexpr T raise1Offset = -100;
    static constexpr T raise2Max = 1;
    static constexpr T raise2Delta = 0.03;
    static constexpr T raise2Offset = 100;
    static constexpr T gaussMax = 0;
    static constexpr T gaussDelta = 1;
    static constexpr T gaussOffset = 0;
    static constexpr T scale = 0.38;
};

template< typename T, template <typename U> class T_Values >
struct GenData1
{
    using Values = T_Values<T>;

    static constexpr T midPt = 1500;
    static constexpr T radius = 21200;
    static constexpr T x0 = midPt - radius;
    static constexpr T y0 = 512u;
    static constexpr T z0 = y0;

    T f(T x, T y, T z) const{
        int dx = x0 - x;
        int dy = y0 - y;
        int dz = z0 - z;
        unsigned r2 = dx*dx + dy*dy + dz*dz;
        T r = std::sqrt(r2) + x0;
        return Values::scale *
                raiseFunc<T>(r, midPt + Values::raise1Offset, Values::raise1Max, Values::raise1Delta) *
                (1 - raiseFunc<T>(r, midPt + Values::raise2Offset, Values::raise2Max, Values::raise2Delta)) *
                (1 - gauss<T>(r, midPt +Values::gaussOffset, Values::gaussMax, Values::gaussDelta));
    }

    template< class T_Idx >
    T
    operator()(T_Idx&& idx) const{
        static constexpr unsigned numDims = LiFFT::traits::NumDims<T_Idx>::value;
        static_assert(numDims == 3, "3D only");

        unsigned x = idx[2] + 850u;
        unsigned y = idx[1];
        unsigned z = idx[0];

        int dx = x0 - x;
        int dy = y0 - y;
        int dz = z0 - z;
        unsigned r2 = dx*dx + dy*dy + dz*dz;
        T r = std::sqrt(r2);
        T rand = T(std::rand()) / RAND_MAX;
        T rand2 = T(std::rand()) / RAND_MAX;
        T rand3 = T(std::rand()) / RAND_MAX;

        T res =  f(x, y, z);
        if(r < radius - 200)
            res += 0.07 *
                sinFunc<T>(x, midPt - 200, 1, 1/(30. + rand3)) *
                sinFunc<T>(y, 0, 1, 1/(120. + rand2)) * sinFunc<T>(y, 0, 1, 1/(120. + rand2)) *
                sinFunc<T>(z, 0, 1,1/(120. + rand)) * sinFunc<T>(z, 0, 1,1/(120. + rand));
        return res;
    }
};

template< class T >
void genData(T& data, unsigned dataSet)
{
    std::cout << "Generating Dataset " << dataSet << "..." << std::endl;
    switch(dataSet){
    case 1:
        LiFFT::generateData(data, GenData1<float, Values1_2P5um>());
        break;
    case 2:
        LiFFT::generateData(data, GenData1<float, Values1_1um>());
        break;
    case 3:
        LiFFT::generateData(data, GenData1<float, Values1_100nm>());
        break;
    case 4:
        LiFFT::generateData(data, GenData1<float, Values2>());
        break;
    default:
        throw std::logic_error("Wrong dataset");
    }
    std::cout << "Generating done" << std::endl;
}

void writeInput(const string& filePath, unsigned dataSet)
{
    tiffWriter::FloatImage<> img(filePath, 1024u, 1024u);
    LiFFT::mem::RealContainer<3, float> data(LiFFT::types::Vec3(1u, 1024u, 1024u));
    unsigned startDS = dataSet ? dataSet : 1;
    unsigned lastDS = dataSet ? dataSet : 4;
    for(unsigned i = startDS; i<=lastDS; i++)
    {
        genData(data, i);
        auto view = LiFFT::types::makeSliceView<0>(data, LiFFT::types::makeRange());
        LiFFT::policies::copy(view, img);
        if(dataSet)
            img.save();
        else
        {
            boost::filesystem::path fPath(filePath);
            fPath.replace_extension(std::to_string(i) + fPath.extension().string());
            img.saveTo(fPath.string());
            img.flush();
        }
    }
}

void writeAllInput(const string& filePath, unsigned dataSet)
{
    tiffWriter::FloatImage<> img(filePath, 1024u, 1024u);
    LiFFT::mem::RealContainer<3, float> data(LiFFT::types::Vec3(1024u, 1024u, 1024u));
    unsigned startDS = dataSet ? dataSet : 1;
    unsigned lastDS = dataSet ? dataSet : 4;
    for(unsigned i = startDS; i<=lastDS; i++)
    {
        genData(data, i);
        for(unsigned z=0; z<data.getExtents()[0]; z++){
            auto view = LiFFT::types::makeSliceView<0>(data, LiFFT::types::makeRange(LiFFT::types::Vec3(z, 0u, 0u)));
            LiFFT::policies::copy(view, img);
            boost::filesystem::path fPath(filePath);
            fPath.replace_extension(std::to_string(i) + string("_") + std::to_string(z) + fPath.extension().string());
            img.saveTo(fPath.string());
            img.flush();
        }
    }
}

void writeFFT(const string& filePath, unsigned dataSet)
{
    using FFT = LiFFT::FFT_3D_R2C_F<true>;
    auto input = FFT::createNewInput(LiFFT::types::Vec3(1024u, 1024u, 1024u));
    auto output = FFT::createNewOutput(input);
    auto outSlice = LiFFT::types::makeSliceView<0>(LiFFT::getFullData(output), LiFFT::types::makeRange());
    auto fft = LiFFT::makeFFT<FFT_LIB, false>(input);

    unsigned startDS = dataSet ? dataSet : 1;
    unsigned lastDS = dataSet ? dataSet : 4;
    for(unsigned i = startDS; i<=lastDS; i++)
    {
        genData(input, i);
        fft(input);
        tiffWriter::FloatImage<> img(filePath, 1024u, 1024u);
        auto acc1 = LiFFT::accessors::makeTransformAccessorFor(LiFFT::policies::CalcIntensityFunc(), outSlice);
        LiFFT::accessors::TransposeAccessor<decltype(acc1)> acc(acc1);
        LiFFT::policies::copy(outSlice, img, acc);
        if(dataSet)
            img.save();
        else
        {
            boost::filesystem::path fPath(filePath);
            fPath.replace_extension(std::to_string(i) + fPath.extension().string());
            img.saveTo(fPath.string());
            img.flush();
        }
    }
}

int
main(int argc, char** argv)
{
    unsigned dataSet;
    string inFilePath, outFilePath;
    unsigned inOrOut;
    options::desc.add_options()
        ("help,h", "Show help message")
        ("outputFile,o", po::value<string>(&outFilePath)->default_value("output.tif"), "Output file to write to")
        ("dataSet,d", po::value<unsigned>(&dataSet)->default_value(0), "Data set to use (1-4) 0 => all")
        ("type,t", po::value<unsigned>(&inOrOut)->default_value(0), "Write Output(0), Input(1) or all Input(2)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options::desc), vm);
    po::notify(vm);

    if (vm.count("help") || outFilePath.empty() || dataSet > 4 )
    {
        showHelp();
        return 1;
    }

    if(inOrOut == 0)
        writeFFT(outFilePath, dataSet);
    else if(inOrOut == 1)
        writeInput(outFilePath, dataSet);
    else
        writeAllInput(outFilePath, dataSet);
}
