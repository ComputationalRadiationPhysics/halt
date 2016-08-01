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
#include "liFFT/types/SymmetricWrapper.hpp"
#include "liFFT/accessors/TransformAccessor.hpp"
#include "liFFT/accessors/TransposeAccessor.hpp"
#include "liFFT/policies/CalcIntensityFunctor.hpp"
#include "liFFT/types/View.hpp"
#include "liFFT/types/SliceView.hpp"
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <string>

namespace po = boost::program_options;
using std::string;
using std::cout;
using std::cerr;

using FP_Type = float;
using ImgType = typename tiffWriter::GetMonochromeImageType<FP_Type>::type;

string
replace(string str, const string& from, const string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return str;
    return str.replace(start_pos, from.length(), to);
}

namespace options{
    po::options_description desc("Convert tiff images via FFT and stores the intensities in another tiff");
} // namespace options

void showHelp()
{
    cout << options::desc << std::endl;
}

void
do2D_FFT(const string& inFilePath, const string& outFilePath)
{
    using namespace LiFFT;
    using FFT = FFT_2D_R2C<FP_Type>;
    auto input = FFT::wrapInput(ImgType(inFilePath, false));
    auto output = FFT::createNewOutput(input);
    auto fft = makeFFT<FFT_LIB, false>(input, output);
    input.getBase().load();
    fft(input, output);
    tiffWriter::FloatImage<> outImg(outFilePath, input.getBase().getWidth(), input.getBase().getHeight());
    auto fullOutput = types::makeSymmetricWrapper(output, input.getExtents()[1]);
    auto transformAcc = accessors::makeTransposeAccessor(
                            accessors::makeTransformAccessorFor(policies::CalcIntensityFunc(), fullOutput)
                        );
    policies::copy(fullOutput, outImg, transformAcc);
    outImg.save();
}

string
getFilledNumber(unsigned num, unsigned minSize, char filler)
{
    string s(std::to_string(num));
    while(s.size()<minSize)
        s = filler + s;
    return s;
}

int
main(int argc, char** argv)
{
    unsigned firstIdx, lastIdx, minSize, x0, y0, actualSize;
    int size;
    char filler;
    string inFilePath, outFilePath;
    options::desc.add_options()
        ("help,h", "Show help message")
        ("inputFile,i", po::value<string>(&inFilePath), "Input file to use, can contain %i as a placeholder for 3D FFTs")
        ("outputFile,o", po::value<string>(&outFilePath)->default_value("output.tif"), "Output file to write to")
        ("firstIdx", po::value<unsigned>(&firstIdx)->default_value(0), "first index to use")
        ("lastIdx", po::value<unsigned>(&lastIdx)->default_value(0), "last index to use")
        ("minSize,m", po::value<unsigned>(&minSize)->default_value(0), "Minimum size of the string replaced for %i")
        ("fillChar,f", po::value<char>(&filler)->default_value('0'), "Char used to fill the string to the minimum size")
        ("xStart,x", po::value<unsigned>(&x0)->default_value(0), "Offset in x-Direction")
        ("yStart,y", po::value<unsigned>(&y0)->default_value(0), "Offset in y-Direction")
        ("size,s", po::value<int>(&size)->default_value(-1), "Size of the image to use (-1=all)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options::desc), vm);
    po::notify(vm);

    if (vm.count("help") || !vm.count("inputFile") || inFilePath.empty() || outFilePath.empty())
    {
        showHelp();
        return 1;
    }

    if(firstIdx > lastIdx)
    {
        std::cerr << "FirstIdx must not be greater then lastIdx" << std::endl;
        return 1;
    }

    if(firstIdx == lastIdx || inFilePath.find("%i") == string::npos)
    {
        // Only 1 image --> 2D FFT
        inFilePath = replace(inFilePath, "%i", getFilledNumber(firstIdx, minSize, filler));
        do2D_FFT(inFilePath, outFilePath);
        return 0;
    }

    using LiFFT::types::Vec2;
    using LiFFT::types::Vec3;
    using LiFFT::types::makeRange;

    // Multiple images --> 3D FFT
    // Assume all images have the same size --> load the first one to get extents and create FFT Data
    auto start = std::chrono::high_resolution_clock::now();
    string curFilePath = replace(inFilePath, "%i", getFilledNumber(firstIdx, minSize, filler));
    ImgType img(curFilePath);
    if(size < 0 )
        actualSize = std::min(img.getWidth(), img.getHeight());
    else
        actualSize = size;
    std::cout << "Processing " << (lastIdx - firstIdx + 1) << " images with region: [" << x0 << ", " << y0 << "] size " << actualSize << std::endl;
    auto imgView = LiFFT::types::makeView(img, makeRange(Vec2(x0, y0), Vec2(actualSize, actualSize)));
    using FFT = LiFFT::FFT_3D_R2C<FP_Type>;
    auto input = FFT::wrapInput(
                    LiFFT::mem::RealContainer<3, FP_Type>(
                            LiFFT::types::Vec<3>(lastIdx-firstIdx+1, actualSize, actualSize)
                    )
                 );
    auto output = FFT::createNewOutput(input);
    auto diff = std::chrono::high_resolution_clock::now() - start;
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Init done: " << sec.count() << "s" << std::endl;

    // Init FFT, makeRange(LiFFT::types::Origin)
    start = std::chrono::high_resolution_clock::now();
    auto fft = LiFFT::makeFFT<FFT_LIB>(input, output);
    diff = std::chrono::high_resolution_clock::now() - start;
    sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "FFT initialized: " << sec.count() << "s" << std::endl;

    // Now copy all the data into one memory region
    start = std::chrono::high_resolution_clock::now();
    auto inputView = LiFFT::types::makeSliceView<0>(input, makeRange());
    LiFFT::policies::copy(imgView, inputView);
    for(unsigned i=firstIdx+1; i<=lastIdx; ++i)
    {
        curFilePath = replace(inFilePath, "%i", getFilledNumber(i, minSize, filler));
        imgView.getBase().open(curFilePath);
        auto view = LiFFT::types::makeSliceView<0>(input, makeRange(Vec3(i-firstIdx, 0u, 0u)));
        LiFFT::policies::copy(imgView, view);
    }
    img.close();
    diff = std::chrono::high_resolution_clock::now() - start;
    sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Data loaded: " << sec.count() << "s" << std::endl;

    // Do the FFT
    start = std::chrono::high_resolution_clock::now();
    fft(input, output);
    diff = std::chrono::high_resolution_clock::now() - start;
    sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "FFT done: " << sec.count() << "s" << std::endl;

    // Copy the intensities to the img and save it
    start = std::chrono::high_resolution_clock::now();
    tiffWriter::FloatImage<> outImg(outFilePath, actualSize, actualSize);
    auto outView = LiFFT::types::makeSliceView<0>(LiFFT::getFullData(output), makeRange());
    auto acc = LiFFT::accessors::makeTransformAccessorFor(LiFFT::policies::CalcIntensityFunc(), outView);
    auto accImg = LiFFT::accessors::makeTransposeAccessorFor(outImg);
    LiFFT::policies::copy(outView, outImg , acc, accImg);
    outImg.save();
    diff = std::chrono::high_resolution_clock::now() - start;
    sec = std::chrono::duration_cast<std::chrono::seconds>(diff);
    std::cout << "Image saved: " << sec.count() << "s" << std::endl;

    return 0;
}
