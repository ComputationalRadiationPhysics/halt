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

#include <memory>

namespace LiFFT {
namespace libraries {
namespace cuFFT {

    template< typename T_In, typename T_Out, class T_Deleter >
    struct Plan
    {
    private:
        template<typename T>
        struct Deleter
        {
            void
            operator()(T* ptr)
            {
                T_Deleter().free(ptr);
            }
        };
        Plan(const Plan&) = delete;
        Plan& operator=(const Plan&) = delete;
    public:
        cufftHandle handle;
        std::unique_ptr<T_In, Deleter<T_In>> InDevicePtr;
        std::unique_ptr<T_Out, Deleter<T_Out>> OutDevicePtr;

        Plan() = default;
        Plan(Plan&& obj): handle(obj.handle), InDevicePtr(std::move(obj.InDevicePtr)), OutDevicePtr(std::move(obj.OutDevicePtr))
        {
            obj.handle = 0;
        }

        Plan& operator=(Plan&& obj)
        {
            if(this!=&obj)
                return *this;
            handle = obj.handle; obj.handle = 0;
            InDevicePtr = std::move(obj.InDevicePtr);
            OutDevicePtr = std::move(obj.OutDevicePtr);
            return *this;
        }

        ~Plan(){
            if(handle)
                cufftDestroy(handle);
        }
  };

}  // namespace cuFFT
}  // namespace libraries
}  // namespace LiFFT
