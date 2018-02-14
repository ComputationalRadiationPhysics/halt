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

#include "libLiFFT/FFT_Kind.hpp"
#include "libLiFFT/FFT_LibPtrWrapper.hpp"
#include "libLiFFT/libraries/clFFT/policies/Context.hpp"
#include "libLiFFT/libraries/clFFT/policies/OpenCLAllocator.hpp"
#include "libLiFFT/libraries/clFFT/policies/OpenCLMemCpy.hpp"
#include "libLiFFT/libraries/clFFT/policies/Planner.hpp"
#include "libLiFFT/libraries/clFFT/policies/ExecutePlan.hpp"
#include <boost/mpl/placeholders.hpp>

namespace bmpl = boost::mpl;

namespace LiFFT {
namespace libraries {
namespace clFFT {

    /**
     * Specifies that inplace transforms are only used when explicitly requested from the FFT
     */
    struct InplaceExplicit: std::false_type{};
    /**
     * Specifies that inplace transforms should be used if both memories reside on host
     */
    struct InplaceForHost: std::true_type{};

    /// ClFFT without Context API
    struct NoContextAPI: std::false_type{};

    /// ClFFT without Context API
    struct WithContextAPI: std::true_type{};

    /**
     * Wrapper for the OpenCL-Library that executes the FFT on an OpenCL device.
     * The interface is extended to support user-provided OpenCL context and
     * queues which are encapsulated by policies/Context.hpp.
     *
     * Note: Allocation and copy will only occur if the IsDeviceMemory trait returns false for the given container
     *
     * \tparam T_InplacePolicy Either InplaceExplicit or InplaceForHost
     * \tparam T_Allocator Policy to alloc/free memory for the input
     * \tparam T_Copier Policy to copy memory to and from the device
     * \tparam T_FFT_Properties Placeholder that will be replaced by a class containing the properties for this FFT
     */
    template<
        class T_EnableContextAPI,
        class T_InplacePolicy = InplaceForHost,
        class T_Allocator = policies::OpenCLAllocator,
        class T_FFT_Properties = bmpl::_1
    >
    class ClFFT
    {
    private:
        static constexpr bool inplaceForHost = T_InplacePolicy::value;
        using Allocator = T_Allocator;
        template<bool T_AsyncEnabled = false>
        using Copier = policies::OpenCLMemCpy<T_AsyncEnabled>;
        using FFT = T_FFT_Properties;
        using Input = typename FFT::Input;
        using Output = typename FFT::Output;
        using PrecisionType = typename FFT::PrecisionType;
        using Planner =
                policies::Planner<
                    PrecisionType,
                    LiFFT::types::TypePair< Input, Output >,
                    FFT::isInplace,
                    FFT::numDims,
                    FFT::isComplexIn,
                    FFT::isComplexOut,
                    FFT::isAoSIn,
                    FFT::isAoSOut,
                    FFT::isStridedIn,
                    FFT::isStridedOut
                >;
        using ExecutePlan =
                policies::ExecutePlan<
                    PrecisionType,
                    LiFFT::types::TypePair< Input, Output >,
                    FFT::isFwd,
                    FFT::isInplace,
                    FFT::numDims,
                    FFT::isComplexIn,
                    FFT::isComplexOut
                >;
        using PlanType = Plan<Allocator>;
        PlanType plan_;

        using ContextFallback = policies::ContextGlobal<>;
        /// only used when T_EnableContextAPI is true, otherwise optimized out
        ContextFallback context_;

        //ClFFT(ClFFT& ) = delete;
        ClFFT& operator=(const ClFFT&) = delete;

    public:

        ClFFT(ClFFT&&) = default;
        ClFFT& operator=(ClFFT&&) = default;

        /**
         * Constructor for out-of-place FFT using own ClFFT context object.
         */
        template< typename = std::enable_if_t<T_EnableContextAPI::value==false> >
        explicit ClFFT(Input& input, Output& output)
        {
            try{
                Planner()(plan_,
                          input, output,
                          inplaceForHost,
                          Allocator(),
                          context_.context(),
                          context_.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }

        /**
         * Constructor for inplace FFT using own ClFFT context object.
         */
        template< typename = std::enable_if_t<T_EnableContextAPI::value==false> >
        explicit ClFFT(Input& inOut)
        {
            try{
                Planner()(plan_,
                          inOut,
                          Allocator(),
                          context_.context(),
                          context_.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }

        /**
         * Executes out-of-place FFT.
         */
        template< typename = std::enable_if_t<T_EnableContextAPI::value==false> >
        void operator()(Input& input, Output& output)
        {
            try {
                ExecutePlan()(plan_,
                              input, output,
                              inplaceForHost,
                              Copier<>(),
                              context_.queue());
            }catch(const std::runtime_error& e){
                std::cerr << "Error in ExecutePlan()() occurred (outplace): "<<e.what()<<std::endl;
                plan_.cleanup();
            }
        }

        /**
         * Executes inplace FFT.
         */
        template< typename = std::enable_if_t<T_EnableContextAPI::value==false> >
        void operator()(Input& inOut)
        {
            try {
                ExecutePlan()(plan_,
                              inOut,
                              Copier<>(),
                              context_.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }

        // --- context API ---

        /**
         * Constructor for out-of-place FFT using a given ClFFT context.
         */
        template< typename T_Context, typename = std::enable_if_t<T_EnableContextAPI::value> >
        explicit ClFFT(Input& input, Output& output, T_Context& ctx)
        {
            try{
                Planner()(plan_,
                          input, output,
                          inplaceForHost,
                          Allocator(),
                          ctx.context(),
                          ctx.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }

        /**
         * Constructor for inplace FFT using a given ClFFT context.
         */
        template< typename T_Context, typename = std::enable_if_t<T_EnableContextAPI::value> >
        explicit ClFFT(Input& inOut, T_Context& ctx)
        {
            try{
                Planner()(plan_,
                          inOut,
                          Allocator(),
                          ctx.context(),
                          ctx.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }


        /**
         * Executes out-of-place FFT in a given ClFFT context.
         */
        template< typename T_Context, typename = std::enable_if_t<T_EnableContextAPI::value> >
        void operator()(Input& input, Output& output, T_Context& ctx)
        {
            try {
                ExecutePlan()(plan_,
                              input, output,
                              inplaceForHost,
                              Copier<T_Context::AsyncEnabled>(),
                              ctx.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }

        template< typename T_Context, typename = std::enable_if_t<T_EnableContextAPI::value> >
        void operator()(Input& inOut, T_Context& ctx)
        {
            try {
                ExecutePlan()(plan_,
                              inOut,
                              Copier<T_Context::AsyncEnabled>(),
                              ctx.queue());
            }catch(const std::runtime_error& e){
                plan_.cleanup();
                throw e;
            }
        }
    };

    using ClFFTNoContextAPI = ClFFT<NoContextAPI>;
    using ClFFTContextAPI = ClFFT<WithContextAPI>;

}  // namespace clFFT
}  // namespace libraries
}  // namespace LiFFT
