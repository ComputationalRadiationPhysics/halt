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

#include "clfft_helper.hpp"

#include <CL/cl.h>
#include <clFFT.h>
#include <memory>

namespace LiFFT {
namespace libraries {
namespace clFFT {

    template<class T_Deleter>
    struct Plan
    {
    private:

        struct Deleter
        {
            void operator()(cl_mem ptr)
            {
                T_Deleter().free(ptr);
            }
        };

        Plan(const Plan&) = delete;
        Plan& operator=(const Plan&) = delete;

    public:
        /// clfft must be initialized before, see policies::Context
        clfftPlanHandle handle = 0;
        std::unique_ptr<_cl_mem, Deleter> InDevicePtr;
        std::unique_ptr<_cl_mem, Deleter> OutDevicePtr;

        Plan()
        {
        }

        Plan(Plan&& obj)
                : handle(obj.handle), InDevicePtr(std::move(obj.InDevicePtr)),
                  OutDevicePtr(std::move(obj.OutDevicePtr))
        {
            obj.handle = 0;
        }

        Plan& operator=(Plan&& obj)
        {
            if(this != &obj)
                return *this;
            handle = obj.handle;
            obj.handle = 0;
            InDevicePtr = std::move(obj.InDevicePtr);
            OutDevicePtr = std::move(obj.OutDevicePtr);
            return *this;
        }

        void cleanup()
        {
            if(handle) {
                CHECK_CL(clfftDestroyPlan(&handle));
                handle = 0;
            }
        }
        ~Plan()
        {
            cleanup();
        }
    };

}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
