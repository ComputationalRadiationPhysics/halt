# libLiFFT
**lib**rary for **L**ibrary-**i**ndependent **FFT**s

[![Build Status](https://travis-ci.org/ComputationalRadiationPhysics/liFFT.svg?branch=master)](https://travis-ci.org/ComputationalRadiationPhysics/liFFT)
[![codecov.io](https://codecov.io/github/ComputationalRadiationPhysics/liFFT/coverage.svg?branch=master)](https://codecov.io/github/ComputationalRadiationPhysics/liFFT?branch=master)

This library contains a generic FFT interface that relies on C++11 features and template metaprogramming to do lots of compile time checks on the validity of the input. It also dynamically compiles only the used library code, which makes it possible to include libraries headers that are not installed on the system, as those are not used in that case.
The currently implemented libraries are:

1. FFTW
2. CuFFT
3. [ClFFT](https://github.com/ComputationalRadiationPhysics/liFFT/pull/26)

For convenience it also contains a generic interface on top of libTIFF to read and write to the following sorts of Tiff images:

1. 24Bit RGB
2. 32Bit ARGB
3. Single and double precision floating point (monochrome)

Almost all possible formats are converted to floating point or 32Bit ARGB images during loading. This allows importing and exporting files from analytic tools and image editors like jImage.

# Software License
**libLiFFT** is licensed under **LGPLv3** or later.

# Namespace organization
All FFT related classes and methods are under the LiFFT namespace, the tiffWriter interface classes are under the tiffWriter namespace. The folder structure matches the namespace organization with the include folder being the top directory.

# Examples
There are a couple of example applications that can be used as a reference for implementing own applications. All of them are in a separate folder:

- test   
    This contains tests for almost all classes of the library and is therefore a good reference of their usage.
- tiff2PDF   
    This is a mini-Example that reads a TIFF image, converts it to a monochrome floating point representation and prints its values to a text file. It then calls a python script to generate a color coded 3D representation of the data stored in pdf format.   
    This can be used for images that show the intensity (abs-squared values of the complex FFT input/output) in a way that is easy to grasp.
- fftTiffImg   
    This is the most complete example. It can convert 2D and 3D datasets of TIFF images via FFT and stores the intensity distribution in another TIFF image.   
    For 2D FFTs a single image has to be specified, for 3D a series of images is required (e.g. data01.tiff, data02.tiff, data03.tiff,...) where each image represents one layer of the volume. Example for images from data004.tiff to data666.tiff: `fftTiffImg -i data%i.tiff --firstIdx=4 --lastIdx=666 -m 3 -f 0`
- reportVolumes   
    Creates 3D data in memory and transforms it, storing the first slice as a TIFF image.
    
# Quickstart

- Include (at least) 'FFT.hpp'
- Choose one of the predefined FFT types from FFT\_Definitions and typedef it to your liking (e.g. `using MyFFT = FFT_2D_C2C`)
- Choose either LiFFT::mem::RealContainer or LiFFT::mem::ComplexContainer for the input data, and set the dimensionality and precision via the template parameters    **OR**
- Use the static member function *MyFFT::createNewInput* to have an appropriate container automatically chosen for you (preferred method)
- Use the *Container::IdxType member as the index type to set the elements via ()-operator (e.g. `Container::IdxType idx(5, 3, 2); container(idx) = 1337;`)
- (If you used a custom container you have to wrap it with `auto input = MyFFT::wrapInput(container)`)
- Call the static member function `MyFFT::createNewOutput(input)` to create an output container (no memory is allocated for inplace transforms) or wrap an existing DataContainer with `auto output = MyFFT::wrapInput(containerOut)`
- Once you got an input and (for non-inplace FFTs) an output container you can call *makeFFT* with the specific FFT library class as the first template parameter and your wrapped container(s) as actual parameters.   
    *Note:* The resulting functor instance contains a so called **plan** and its creation requires time and memory. It is meant to be reused for optimal performance!
- Now everything is set up and you only need to call the created FFT functor (gotten by *makeFFT*) with the wrapped container(s) to execute an FFT. This process can be repeated with the same containers (and different data contained within) as often as required.
- The result can be accessed via the output container. Note that for inplace transforms the output container contains only a reference to the input container, so it must not get out of scope before you are finished accessing the output.

A good example is the code from fftTiffImg:
```C++
void
do2D_FFT(const string& inFilePath, const string& outFilePath)
{
    using namespace LiFFT;
    using FFT = FFT_2D_R2C_F<>;
    auto input = FFT::wrapInput(tiffWriter::FloatImage<>(inFilePath, false));
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
```    
# Usage notes

- The design relies on containers, accessors and indices:
    - A container is a data storage together with meta-data (dimensionality, extents, strides, types...). Usually a container provides access to the data via a ()- or []-operator. In general the identity accessor can be used for a uniform access.
    - An accessor is used to access the data in a container. That is done by calling the accessor functor with (index, container). Each container has an identity accessor, that returns (and writes, if possible) the plain data in the container. It is also possible, that a different accessor modifies the index or the data on access (e.g. TransformAccessor or TransposeAccessor)
    - An index is a n-dimensional vector used to refer to a specific element. The fastest varying dimension (mostly the 'x'-dimension in C/C++) is the last element. So the 2nd element in x direction and the 3rd element in y direction can by accessed by (2, 1). This is important for loops where the innermost loop should loop over the last index entry.
- If you want only a specific part of a container, you can use the View or the SliceView. The SliceView additionally strips of 1 dimension of the container (so it becomes a 2D 'Slice' of a 3D container)
- All properties/meta data can be queried by traits or policies. Traits return compiletime constants (e.g. dimensionality) where policies return runtime information (e.g. extents) If you use own data containers, you have to specialize a couple of traits, to make it work together with other pieces of the library. You can also define class constants and typedefs with specific names to use the default specialization (e.g. *numDims*, *isComplex* or *IdentityAccessor*) Check the traits implementation for the default names. 
- makeFFT requires allocated memory. It treats it as read-only unless the 2nd template parameter is set to false. Doing so may improve execution speed of the FFT to the cost of longer creation time and also destroying the data in the input/output given. So call it **before** filling the data

# Naming convention

In general the following naming convention was used
- Types: UpperCamelCase
- Variables: lowerCamelCase
- private member variables: m_lowerCamelCase (to avoid shadow warnings)
- shadowing parameters: lowerCamelCaseIn (for *In*put)
- Type template parameters: T_UpperCamelCase
- Non-Type template parameters: T_lowerCamelCase

