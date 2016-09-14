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
#include <iostream>

namespace LiFFT {
namespace libraries {
namespace clFFT {
namespace policies {

  struct ClFFTContext {
    bool initialized = false;
    /**
     * Setting up clfft context
     */
    void init() {
      clfftSetupData fftSetup;
      CHECK_CL( clfftInitSetupData(&fftSetup) );
      CHECK_CL( clfftSetup(&fftSetup) );
      initialized = true;
    }
    /**
     * Destroying clfft context if it was initialized
     */
    ~ClFFTContext() {
      if(initialized)
        CHECK_CL(clfftTeardown());
    }
  };

  /**
   * Base class with OpenCL context and device pointer
   */
  struct Context {
    cl_context ctx_ = 0;
    cl_device_id dev_ = 0;
    cl_command_queue queue_ = 0;
  };

  /**
   * Provides OpenCL context and device after instantiation, where clfft is
   * initialized as well.
   *
   * If no GPU is found, CPUs are used by OpenCL. For own OpenCL context and
   * device, use the ContextWrapper class.
   */
  class ContextLocal : private Context {
  public:
    ClFFTContext clfft;
    cl_context context() {
      return ctx_;
    }
    cl_device_id device() {
      return dev_;
    }
    cl_command_queue queue() {
      return queue_;
    }
    ContextLocal()
    {
      cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
      cl_platform_id platform = 0;
      cl_int err = 0;
      findClDevice(CL_DEVICE_TYPE_GPU, &platform, &dev_);
      props[1] = (cl_context_properties)platform;
      ctx_ = clCreateContext(props, 1, &dev_, NULL, NULL, &err);
      CHECK_CL(err);
      clfft.init();
      queue_ = clCreateCommandQueue( ctx_, dev_, 0, &err );
      CHECK_CL(err);
    }
    ~ContextLocal()
    {
      CHECK_CL(clReleaseCommandQueue( queue_ ));
      queue_ = 0;
      CHECK_CL(clReleaseContext( ctx_ ));
      ctx_ = 0;
    }
  };

/**
 * A singleton by using a single ContextLocal instance.
 */
  class ContextGlobal
  {
  private:
    static ContextLocal* getInstance()
    {
      static ContextLocal instance;
      return &instance;
    }
  public:
    cl_context context()
    {
      return getInstance()->context();
    }
    cl_device_id device()
    {
      return getInstance()->device();
    }
    cl_command_queue queue() {
      return getInstance()->queue();
    }
  };

/**
 * Wraps OpenCL context and device given by user.
 */
  class ContextWrapper
  {
  public:
    static void wrap(cl_context c,
                     cl_device_id d,
                     bool create_clfft_context)
    {
      wrap(c, d, 0, create_clfft_context);
    }
    static void wrap(cl_context c,
                     cl_device_id d,
                     cl_command_queue q,
                     bool create_clfft_context)
    {
      static ClFFTContext clfft;
      if(create_clfft_context)
        clfft.init();
      context_static(c);
      device_static(d);
      if(q!=0)
        queue_static(q);
    }
    static cl_context context_static(cl_context c=0) {
      static cl_context ctx; // init
      if(c!=0) ctx=c; // re-assign
      return ctx;
    }
    static cl_device_id device_static(cl_device_id d=0) {
      static cl_device_id dev; // init
      if(d!=0) dev=d; // re-assign
      return dev;
    }
    static cl_command_queue queue_static(cl_command_queue q=0) {
      static cl_command_queue queue; // init
      if(q!=0) queue=q; // re-assign
      return queue;
    }
    cl_context context() {
      return context_static();
    }
    cl_device_id device() {
      return device_static();
    }
    cl_command_queue queue() {
      return queue_static();
    }
  };
}
}
}
}

#endif /* CONTEXT_HPP_ */
