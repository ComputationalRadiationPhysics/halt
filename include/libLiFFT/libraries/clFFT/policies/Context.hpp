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
#ifndef CONTEXT_HPP_
#define CONTEXT_HPP_
#include "libLiFFT/libraries/clFFT/clfft_helper.hpp"
#include <clFFT.h>
#include <boost/algorithm/string.hpp> // split, is_any_of
#include <iostream>
#include <regex>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

    /**
     * This Singleton takes care, that only one clfft setup is done, otherwise
     * errors might happen at clfft execution (invalid plans) due to multiple
     * instances of clfft at the same time.
     */
    struct ClFFTLibrary
    {
        static ClFFTLibrary& getInstance()
        {
            static ClFFTLibrary clfft;
            return clfft;
        }

        bool ready()
        {
            return initialized;
        }

    private:

        bool initialized = false;

        /**
         * Setting up clfft context
         */
        ClFFTLibrary()
        {
            clfftSetupData fftSetup;
            try {
                CHECK_CL(clfftInitSetupData(&fftSetup));
                CHECK_CL(clfftSetup(&fftSetup));
                initialized = true;
            }
            catch(const std::runtime_error&) {
            }
        }

        /**
         * Destroying clfft context if it was initialized
         */
        ~ClFFTLibrary()
        {
            if(initialized)
                CHECK_CL(clfftTeardown());
        }
    };

    /**
     * Base class with OpenCL context and device pointer
     */
    struct Context
    {
        cl_context ctx_ = 0;
        cl_device_id dev_ = 0;
        cl_command_queue queue_ = 0;
    };

    enum class ContextDevice {
        GPU=CL_DEVICE_TYPE_GPU,
        CPU=CL_DEVICE_TYPE_CPU,
        ACCELERATOR=CL_DEVICE_TYPE_ACCELERATOR
    };

    /**
     * Provides OpenCL context and device after instantiation, where clfft is
     * initialized as well.
     *
     * If no GPU is found, CPUs are used by OpenCL. For own OpenCL context and
     * device, use the ContextWrapper class.
     */
    template<bool T_Async=false>
    class ContextLocal : private Context
    {
    public:

        constexpr static bool AsyncEnabled = T_Async;

        cl_context context()
        {
            return ctx_;
        }

        cl_device_id device()
        {
            return dev_;
        }

        cl_command_queue queue()
        {
            return queue_;
        }

        template< typename T=int, typename std::enable_if<AsyncEnabled,T>::type = 0 >
        void sync_queue()
        {
            CHECK_CL(clFinish(queue_));
        }

        ContextLocal()
        : ContextLocal(ContextDevice::GPU)
        {
        }

        explicit ContextLocal(const ContextDevice _devtype) {
            if(ClFFTLibrary::getInstance().ready()) {

                cl_int err = 0;
                cl_platform_id platform = 0;
                cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
                cl_device_type devtype = static_cast<cl_device_type>(_devtype);
                findClDevice(devtype, &platform, &dev_);
                props[1] = reinterpret_cast<cl_context_properties>(platform);
                ctx_ = clCreateContext(props, 1, &dev_, nullptr, nullptr, &err);
                CHECK_CL(err);
                queue_ = clCreateCommandQueue(ctx_, dev_, 0, &err);
                CHECK_CL(err);
            }
            else
                throw std::runtime_error("Could not initialize clfft library.");
        }

        explicit ContextLocal(const std::string dev_str) {

            std::regex e("^([0-9]+):([0-9]+)$"); // get user specified platform and device id
            if(std::regex_search(dev_str, e)) {
                cl_int err = 0;
                cl_platform_id platform = 0;
                cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
                std::vector<std::string> token;
                boost::split(token, dev_str, boost::is_any_of(":"));
                unsigned long id_platform = std::stoul(token[0].c_str());
                unsigned long id_device = std::stoul(token[1].c_str());
                getPlatformAndDeviceByID(&platform,
                                       &dev_,
                                       id_platform,
                                       id_device);
                props[1] = reinterpret_cast<cl_context_properties>(platform);
                ctx_ = clCreateContext(props, 1, &dev_, nullptr, nullptr, &err);
                CHECK_CL(err);
                queue_ = clCreateCommandQueue(ctx_, dev_, 0, &err);
                CHECK_CL(err);
            } else
                ContextLocal();

        }

        ~ContextLocal()
        {
            if(queue_) {
                CHECK_CL(clReleaseCommandQueue(queue_));
                queue_ = 0;
            }
            if(ctx_) {
                CHECK_CL(clReleaseContext(ctx_));
                ctx_ = 0;
            }
        }
    };

    /**
     * A singleton by using a single ContextLocal instance.
     */
    template<bool T_Async=false>
    class ContextGlobal
    {
    private:

        static ContextLocal<T_Async>& getInstance()
        {
            static ContextLocal<T_Async> instance;
            return instance;
        }

    public:

        constexpr static bool AsyncEnabled = T_Async;

        cl_context context()
        {
            return getInstance().context();
        }

        cl_device_id device()
        {
            return getInstance().device();
        }

        cl_command_queue queue()
        {
            return getInstance().queue();
        }

        template< typename T=int, typename std::enable_if<AsyncEnabled,T>::type = 0 >
        void sync_queue()
        {
            CHECK_CL(clFinish(queue()));
        }
    };

    /**
     * Wraps OpenCL context and device given by user.
     */
    template<bool T_Async=false>
    class ContextWrapper
    {
    public:

        constexpr static bool AsyncEnabled = T_Async;
        cl_context ctx_ = nullptr;
        cl_command_queue queue_ = nullptr;
        cl_device_id dev_ = nullptr;

        explicit ContextWrapper(cl_context c, cl_device_id d, cl_command_queue q)
        {
            if(!ClFFTLibrary::getInstance().ready())
                throw std::runtime_error("Could not initialize clfft library.");
            assert(c!=nullptr && d!=nullptr && q!=nullptr);
            ctx_ = c;
            dev_ = d;
            queue_ = q;
        }

        cl_context context()
        {
            return ctx_;
        }

        cl_device_id device()
        {
            return dev_;
        }

        cl_command_queue queue()
        {
            return queue_;
        }

        template< typename T=int, typename std::enable_if<AsyncEnabled,T>::type = 0 >
        void sync_queue()
        {
            CHECK_CL(clFinish(queue()));
        }
    };
}
}
}
}

#endif /* CONTEXT_HPP_ */
