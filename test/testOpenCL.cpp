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
#include "testDefines.hpp"
#include "libLiFFT/generateData.hpp"
#include "libLiFFT/FFT.hpp"
#include "libLiFFT/FFT_LibPtrWrapper.hpp"

#include "libLiFFT/libraries/clFFT/clfft_helper.hpp"
#include "libLiFFT/libraries/clFFT/policies/Context.hpp"
#include <CL/cl.h>

#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/int.hpp>

#include <chrono>

using LiFFT::generateData;
using namespace LiFFT::generators;

using ClFFTContextAPI= LiFFT::libraries::clFFT::ClFFTContextAPI;

struct TimerCPU {
  typedef std::chrono::high_resolution_clock clock;

  clock::time_point start;
  double time = 0.0;

  void startTimer() {
    start = clock::now();
  }

  double stopTimer() {
    auto diff = clock::now() - start;
    return (time = std::chrono::duration<double, std::milli> (diff).count());
  }
};


namespace LiFFTTest {

    BOOST_AUTO_TEST_SUITE(OpenCL)

    namespace utf = boost::unit_test;
    using namespace LiFFT::libraries::clFFT;
    using namespace LiFFT::libraries::clFFT::policies;

    // checks if global fixture works
    BOOST_AUTO_TEST_CASE(TestInit)
    {
        BOOST_CHECK(true);
    }

    BOOST_AUTO_TEST_CASE(ListDevices)
    {
        std::cout << listClDevices();
    }


    BOOST_AUTO_TEST_CASE(TestContextLocal)
    {
        ContextLocal<> context_local;
        BOOST_CHECK(context_local.context());
        BOOST_CHECK(context_local.queue());
    }

    BOOST_AUTO_TEST_CASE(TestContextGlobal)
    {
        ContextGlobal<> context_global;
        BOOST_CHECK(context_global.context());
        BOOST_CHECK(context_global.queue());
    }

    BOOST_AUTO_TEST_CASE(TestContextWrapper)
    {
        cl_platform_id platform = 0;
        cl_int err = 0;
        cl_context ctx = 0;
        cl_device_id dev = 0;
        cl_command_queue queue = 0;
        findClDevice(CL_DEVICE_TYPE_GPU, &platform, &dev);
        ctx = clCreateContext(NULL, 1, &dev, NULL, NULL, &err);
        CHECK_CL(err);
        queue = clCreateCommandQueue( ctx, dev, 0, &err );
        CHECK_CL(err);

        ContextWrapper<> context(ctx, dev, queue);
        BOOST_CHECK(context.context());
        BOOST_CHECK(context.queue());

        CHECK_CL(clReleaseCommandQueue( queue ));
        CHECK_CL(clReleaseContext( ctx ));
    }

    BOOST_AUTO_TEST_CASE(TestC2CRawCLFFT)
    {

        using FFT = LiFFT::FFT_2D_C2C<TestPrecision>;
        auto aperture = ComplexContainer(TestExtents::all(testSize));
        auto input = FFT::wrapInput(aperture);
        generateData(input, Rect<TestPrecision>(20,testSize/2));
        LiFFT::policies::copy(aperture, baseC2CInput);

        ContextGlobal<> context;
        auto queue = context.queue();
        cl_int err;
        cl_mem dat1 = clCreateBuffer(context.context(),
                                     CL_MEM_READ_WRITE,
                                     baseC2CInput.getMemSize(), nullptr, &err);
        CHECK_CL(err);
        cl_mem dat2 = clCreateBuffer(context.context(),
                                     CL_MEM_READ_WRITE,
                                     baseC2COutput.getMemSize(), nullptr, &err);

        CHECK_CL(err);
        CHECK_CL(clEnqueueWriteBuffer( queue,
                                       dat1,
                                       CL_TRUE,
                                       0,
                                       baseC2CInput.getMemSize(),
                                       baseC2CInput.getData(),
                                       0, NULL, NULL ));

        clfftPlanHandle handle = 0;
        size_t cldims[2] = {testSize, testSize};

        CHECK_CL(clfftCreateDefaultPlan(&handle, context.context(), CLFFT_2D, cldims));
        CHECK_CL(clfftSetPlanPrecision(handle, CLFFT_SINGLE));
        CHECK_CL(clfftSetLayout(handle,
                                CLFFT_COMPLEX_INTERLEAVED,
                                CLFFT_COMPLEX_INTERLEAVED));
        CHECK_CL(clfftSetResultLocation(handle, CLFFT_OUTOFPLACE));
        CHECK_CL(clfftBakePlan(handle, 1, // number of queues
                               &queue, nullptr, // callback
                               nullptr)); // user data
        CHECK_CL(clFinish(queue));
        CHECK_CL(
                clfftEnqueueTransform(handle, CLFFT_FORWARD, 1, &queue, // queue
                0, NULL, NULL,// events
                &dat1,// input buffer
                &dat2,// output buffer
                NULL// tmp buffer
                ));
        CHECK_CL(clFinish(queue));


        // this wrapped lib ptr is non-accessible (as cl_mem is)
        //auto outWrapped = FFT::wrapOutputLibPtr(dat2, TestExtents(testSize, testSize));

        // so we have to create a host buffer first to copy data into
        using Complex = LiFFT::types::Complex<TestPrecision>;
        std::unique_ptr<Complex[]> output(new Complex[testSize * testSize]);
        CHECK_CL(clEnqueueReadBuffer( queue,
                                      dat2,
                                      CL_TRUE, // blocking_write
                                      0, // offset
                                      baseC2COutput.getMemSize(),
                                      output.get(),
                                      0, // num_events_in_wait_list
                                      nullptr, // event_wait_list
                                      nullptr // event
                                      ));
        auto outWrapped = FFT::wrapOutput(LiFFT::mem::wrapPtr<true>(
                                        output.get(),
                                        TestExtents(testSize, testSize)
                                        ));
        execBaseC2C();

        CHECK_CL(clReleaseMemObject(dat1));
        CHECK_CL(clReleaseMemObject(dat2));

        checkResult(baseC2COutput, outWrapped, "C2C outplace", CmpError(1e-3, 5e-5));
    }

    BOOST_AUTO_TEST_CASE(TestC2CRawDevicePtr)
    {
        using FFT = LiFFT::FFT_2D_C2C<TestPrecision>;
        auto aperture = ComplexContainer(TestExtents::all(testSize));
        auto input = FFT::wrapInput(aperture);
        generateData(input, Rect<TestPrecision>(20, testSize / 2));
        LiFFT::policies::copy(aperture, baseC2CInput);

        ContextGlobal<> context;
        auto queue = context.queue();
        cl_int err;
        cl_mem dat1 = clCreateBuffer(context.context(),
                                     CL_MEM_READ_WRITE,
                                     baseC2CInput.getMemSize(), nullptr, &err);
        CHECK_CL(err);
        cl_mem dat2 = clCreateBuffer(context.context(),
                                     CL_MEM_READ_WRITE,
                                     baseC2COutput.getMemSize(), nullptr, &err);

        CHECK_CL(err);

        CHECK_CL(clEnqueueWriteBuffer( context.queue(),
                                       dat1,
                                       CL_TRUE,
                                       0,
                                       baseC2CInput.getMemSize(),
                                       baseC2CInput.getData(),
                                       0, NULL, NULL ));
        try {

            auto inWrapped = FFT::wrapInputLibPtr(dat1,
                                                  TestExtents(testSize, testSize));
            auto outWrapped = FFT::wrapOutputLibPtr(
                    dat2, TestExtents(testSize, testSize));
            auto fft = LiFFT::makeFFTInQueue<ClFFTContextAPI>(inWrapped, outWrapped,
                                                              context);
            fft(inWrapped, outWrapped, context);
        }
        catch(std::runtime_error& e) {
            BOOST_ERROR(e.what());
        }
        // this wrapped lib ptr is non-accessible (as cl_mem is)
        //auto outWrapped = FFT::wrapOutputLibPtr(dat2, TestExtents(testSize, testSize));

        // so we have to create a host buffer first to copy data into
        using Complex = LiFFT::types::Complex<TestPrecision>;
        std::unique_ptr<Complex[]> output(new Complex[testSize * testSize]);
        CHECK_CL(clEnqueueReadBuffer( queue,
                                      dat2,
                                      CL_TRUE, // blocking_write
                                      0, // offset
                                      baseC2COutput.getMemSize(),
                                      output.get(),
                                      0, // num_events_in_wait_list
                                      nullptr, // event_wait_list
                                      nullptr // event
                                      ));
        auto outWrapped = FFT::wrapOutput(LiFFT::mem::wrapPtr<true>(
                                        output.get(),
                                        TestExtents(testSize, testSize)
                                        ));
        execBaseC2C();

        CHECK_CL(clReleaseMemObject(dat1));
        CHECK_CL(clReleaseMemObject(dat2));

        checkResult(baseC2COutput, outWrapped, "C2C outplace", CmpError(1e-3, 5e-5));

    }

    BOOST_AUTO_TEST_CASE(TestClFFTR2CWithoutContext)
    {

        using Real = LiFFT::types::Real<TestPrecision>;
        using Complex = LiFFT::types::Complex<TestPrecision>;

        std::unique_ptr<Real[]> input(new Real[testSize * testSize]);
        std::unique_ptr<Complex[]> output(
                new Complex[testSize * (testSize / 2 + 1)]);
        for(unsigned i = 0; i < testSize * testSize; ++i) {
            input[i] = std::rand() / RAND_MAX;
        }
        // default case without explicit opencl interaction
        {
            using FFT_TYPE = LiFFT::FFT_2D_R2C<TestPrecision>;
            auto inWrapped = FFT_TYPE::wrapInput(
                    LiFFT::mem::wrapPtr<false>(input.get(),
                                               TestExtents(testSize, testSize)));
            auto outWrapped = FFT_TYPE::wrapOutput(
                    LiFFT::mem::wrapPtr<true>(
                            output.get(), TestExtents(testSize, testSize / 2 + 1)));
            LiFFT::policies::copy(inWrapped, baseR2CInput);

            auto fft = LiFFT::makeFFT<TestLibrary>(inWrapped, outWrapped);
            fft(inWrapped, outWrapped);

            execBaseR2C();
            checkResult(baseR2COutput, outWrapped, "R2C outplace", CmpError(1e-3, 5e-5));
        }
    }

    BOOST_AUTO_TEST_CASE(TestClFFTR2CWithContext)
    {

        using Context = LiFFT::libraries::clFFT::policies::ContextLocal<>;
        using Real = LiFFT::types::Real<TestPrecision>;
        using Complex = LiFFT::types::Complex<TestPrecision>;

        std::unique_ptr<Real[]> input(new Real[testSize * testSize]);
        std::unique_ptr<Complex[]> output(new Complex[testSize * (testSize / 2 + 1)]);
        for(unsigned i = 0; i < testSize * testSize; ++i) {
            input[i] = std::rand() / RAND_MAX;
        }
        // default case without explicit opencl interaction
        {
            Context context;
            using FFT_TYPE = LiFFT::FFT_2D_R2C<TestPrecision>;
            auto inWrapped = FFT_TYPE::wrapInput(
                    LiFFT::mem::wrapPtr<false>(input.get(),
                                               TestExtents(testSize, testSize)));
            auto outWrapped = FFT_TYPE::wrapOutput(
                    LiFFT::mem::wrapPtr<true>(
                            output.get(), TestExtents(testSize, testSize / 2 + 1)));
            LiFFT::policies::copy(inWrapped, baseR2CInput);

            auto fft = LiFFT::makeFFTInQueue<ClFFTContextAPI>(inWrapped,
                                                              outWrapped,
                                                              context);
            fft(inWrapped, outWrapped, context);

            execBaseR2C();
            checkResult(baseR2COutput, outWrapped, "R2C outplace", CmpError(1e-3, 5e-5));
        }
    }

    BOOST_AUTO_TEST_CASE(TestClFFTR2CWithAsyncContext)
    {

        using Context = LiFFT::libraries::clFFT::policies::ContextLocal<true>;
        using Real = LiFFT::types::Real<TestPrecision>;
        using Complex = LiFFT::types::Complex<TestPrecision>;

        std::unique_ptr<Real[]> input(new Real[testSize * testSize]);
        std::unique_ptr<Complex[]> output(new Complex[testSize * (testSize / 2 + 1)]);
        for(unsigned i = 0; i < testSize * testSize; ++i) {
            input[i] = std::rand() / RAND_MAX;
        }
        // default case without explicit opencl interaction
        {
            Context context;
            using FFT_TYPE = LiFFT::FFT_2D_R2C<TestPrecision>;
            auto inWrapped = FFT_TYPE::wrapInput(
                    LiFFT::mem::wrapPtr<false>(input.get(),
                                               TestExtents(testSize, testSize)));
            auto outWrapped = FFT_TYPE::wrapOutput(
                    LiFFT::mem::wrapPtr<true>(output.get(),
                                              TestExtents(testSize, testSize / 2 + 1)));
            LiFFT::policies::copy(inWrapped, baseR2CInput);

            auto fft = LiFFT::makeFFTInQueue<ClFFTContextAPI>(inWrapped,
                                                              outWrapped,
                                                              context);
            fft(inWrapped, outWrapped, context);

            execBaseR2C();

            context.sync_queue(); // if not present, then test fails as copies do not finish in time
            checkResult(baseR2COutput, outWrapped, "R2C outplace", CmpError(1e-3, 5e-5));
        }
    }

    BOOST_AUTO_TEST_CASE(TestClFFTR2CInplace)
    {
        TestExtents ext = TestExtents::all(testSize);
        ext[testNumDims - 1] = (ext[testNumDims - 1] / 2 + 1) * 2;
        using namespace LiFFT::types;
        auto aperture = makeView(RealContainer(ext),
                                 makeRange(Origin(), TestExtents::all(testSize)));
        using FFT_Type = LiFFT::FFT_2D_R2C<TestPrecision, true>;
        auto input = FFT_Type::wrapInput(aperture);
        auto output = FFT_Type::createNewOutput(input);

        generateData(input, Rect<TestPrecision>(20, testSize / 2));
        LiFFT::policies::copy(aperture, baseR2CInput);
        auto fft = LiFFT::makeFFT<TestLibrary>(input);
        fft(input);
        execBaseR2C();
        checkResult(baseR2COutput, output, "R2C inPlace", CmpError(1e-3, 5e-5));
    }

    BOOST_AUTO_TEST_CASE(TestClFFTR2CInplaceTwoArch)
    {
        TimerCPU timer;
        using Context = LiFFT::libraries::clFFT::policies::ContextLocal<>;
        Context context_cpu(ContextDevice::CPU);
        Context context_gpu(ContextDevice::GPU);

        std::cout << "1: " << getClDeviceInformations(context_cpu.device()) << "\n";
        std::cout << "2: " << getClDeviceInformations(context_gpu.device()) << "\n";

        TestExtents ext = TestExtents::all(testSize);
        ext[testNumDims - 1] = (ext[testNumDims - 1] / 2 + 1) * 2;
        using namespace LiFFT::types;
        auto aperture_cpu = makeView(RealContainer(ext),
                                     makeRange(Origin(), TestExtents::all(testSize)));
        auto aperture_gpu = makeView(RealContainer(ext),
                                     makeRange(Origin(), TestExtents::all(testSize)));

        using FFT_Type = LiFFT::FFT_2D_R2C<TestPrecision, true>;
        auto input_cpu = FFT_Type::wrapInput(aperture_cpu);
        auto output_cpu = FFT_Type::createNewOutput(input_cpu);
        auto input_gpu = FFT_Type::wrapInput(aperture_gpu);
        auto output_gpu = FFT_Type::createNewOutput(input_gpu);

        generateData(input_cpu, Rect<TestPrecision>(20, testSize / 2));
        generateData(input_gpu, Rect<TestPrecision>(20, testSize / 2));
        LiFFT::policies::copy(aperture_cpu, baseR2CInput);

        auto fft_cpu = LiFFT::makeFFTInQueue<ClFFTContextAPI>(input_cpu, context_cpu);
        auto fft_gpu = LiFFT::makeFFTInQueue<ClFFTContextAPI>(input_gpu, context_gpu);

        { // warmup
          fft_cpu(input_cpu, context_cpu);
          fft_gpu(input_gpu, context_gpu);
        }

        { // rerun on the data and measure time
          generateData(input_cpu, Rect<TestPrecision>(20, testSize / 2));
          generateData(input_gpu, Rect<TestPrecision>(20, testSize / 2));

          timer.startTimer();
          fft_cpu(input_cpu, context_cpu);
          fft_gpu(input_gpu, context_gpu);
          double ms = timer.stopTimer();
          std::cout << "TwoArch Sync: Time = " << ms << "\n";
        }

        execBaseR2C();
        checkResult(baseR2COutput, output_cpu, "R2C inPlace", CmpError(1e-3, 5e-5));
        checkResult(baseR2COutput, output_gpu, "R2C inPlace", CmpError(1e-3, 5e-5));
    }

    BOOST_AUTO_TEST_CASE(TestClFFTR2CInplaceTwoArchAsync)
    {
        TimerCPU timer;
        using Context = LiFFT::libraries::clFFT::policies::ContextLocal<true>;
        Context context_cpu(ContextDevice::CPU);
        Context context_gpu(ContextDevice::GPU);

        TestExtents ext = TestExtents::all(testSize);
        ext[testNumDims - 1] = (ext[testNumDims - 1] / 2 + 1) * 2;
        using namespace LiFFT::types;
        auto aperture_cpu = makeView(RealContainer(ext),
                                     makeRange(Origin(), TestExtents::all(testSize)));
        auto aperture_gpu = makeView(RealContainer(ext),
                                     makeRange(Origin(), TestExtents::all(testSize)));

        using FFT_Type = LiFFT::FFT_2D_R2C<TestPrecision, true>;
        auto input_cpu = FFT_Type::wrapInput(aperture_cpu);
        auto output_cpu = FFT_Type::createNewOutput(input_cpu);
        auto input_gpu = FFT_Type::wrapInput(aperture_gpu);
        auto output_gpu = FFT_Type::createNewOutput(input_gpu);

        generateData(input_cpu, Rect<TestPrecision>(20, testSize / 2));
        generateData(input_gpu, Rect<TestPrecision>(20, testSize / 2));
        LiFFT::policies::copy(aperture_cpu, baseR2CInput);

        auto fft_cpu = LiFFT::makeFFTInQueue<ClFFTContextAPI>(input_cpu, context_cpu);
        auto fft_gpu = LiFFT::makeFFTInQueue<ClFFTContextAPI>(input_gpu, context_gpu);

        { // warmup
          fft_cpu(input_cpu, context_cpu);
          fft_gpu(input_gpu, context_gpu);
          context_cpu.sync_queue();
          context_gpu.sync_queue();
        }

        { // rerun on the data and measure time
          generateData(input_cpu, Rect<TestPrecision>(20, testSize / 2));
          generateData(input_gpu, Rect<TestPrecision>(20, testSize / 2));

          timer.startTimer();
          fft_cpu(input_cpu, context_cpu);
          fft_gpu(input_gpu, context_gpu);

          context_gpu.sync_queue(); // if not present, then test fails as copies do not finish in time
          context_cpu.sync_queue(); // if not present, then test fails as copies do not finish in time
          double ms = timer.stopTimer();
          std::cout << "TwoArch ASync: Time = " << ms << "\n";
        }

        execBaseR2C();
        checkResult(baseR2COutput, output_cpu, "R2C inPlace", CmpError(1e-3, 5e-5));
        checkResult(baseR2COutput, output_gpu, "R2C inPlace", CmpError(1e-3, 5e-5));
    }

//    BOOST_AUTO_TEST_CASE(TestCopyToLibPtr)
//    {
//
//        using FFT = LiFFT::FFT_2D_C2C<TestPrecision>;
//        auto aperture = ComplexContainer(TestExtents::all(testSize));
//        auto input = FFT::wrapInput(aperture);
//        generateData(input, Rect<TestPrecision>(20,testSize/2));
//        LiFFT::policies::copy(aperture, baseC2CInput);
//
//        ContextGlobal<> context;
//        cl_int err;
//        cl_mem dat1 = clCreateBuffer(context.context(),
//                                     CL_MEM_READ_WRITE,
//                                     baseC2CInput.getMemSize(), nullptr, &err);
//
//        LiFFT::policies::copy(aperture, baseC2CInput);
//        auto inWrapped = FFT::wrapInputLibPtr(dat1,
//                                              TestExtents(testSize, testSize));
//        LiFFT::policies::copy(inWrapped, baseC2CInput);
//        CHECK_CL(clReleaseMemObject(dat1));
//
//    }

    BOOST_AUTO_TEST_SUITE_END()
  
} // namespace LiFFTTest
