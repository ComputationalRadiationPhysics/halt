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

#include "libLiFFT/FFT_Properties.hpp"

// Included for convenience, so only one include is required from user code
#include "libLiFFT/FFT_Definition.hpp"
#include "libLiFFT/FFT_LibPtrWrapper.hpp"
#include "libLiFFT/FFT_DataWrapper.hpp"
#include <boost/mpl/apply.hpp>
#include <type_traits>

namespace bmpl = boost::mpl;

namespace LiFFT {

    struct DefaultQueue {};

    /**
     * Assembles an FFT
     *
     * Usage:
     *      1) The constructor takes the container(s) and may modify the memory
     *         Note: An implementation may not need the memory at all
     *      2) Execute the FFT with <fftInstance>(input, output), which performs the transform
     *         from the memories in the wrappers. If the base accessors of the wrappers do not return a reference type
     *         internal memory is allocated and data is copied before/after the FFT
     *
     * Parameters:
     * \tparam T_Library FFT Library to use
     * \tparam T_InputWrapper   Input wrapped in a FFT_DataWrapper
     * \tparam T_OutputWrapper  Output wrapped in a FFT_DataWrapper
     * \tparam T_constructWithReadOnly If true, the data passed in the constructor is not overwritten. Use false for better performance
     */
    template<
            class T_Library,
            typename T_InputWrapper,
            typename T_OutputWrapper,
            bool T_constructWithReadOnly = true,
            typename T_Queue = DefaultQueue
            >
    class FFT
    {
        using Library = T_Library;
        using Input = T_InputWrapper;
        using Output = T_OutputWrapper;
        static constexpr bool constructWithReadOnly = T_constructWithReadOnly;
        template<typename T>
        using EnableIfQueuePresentAndInplace = typename std::enable_if<!std::is_same<T_Queue,DefaultQueue>::value && T_InputWrapper::FFT_Def::isInplace,T>;
        template<typename T>
        using EnableIfNoQueueAndInplace = typename std::enable_if<std::is_same<T_Queue,DefaultQueue>::value && T_InputWrapper::FFT_Def::isInplace,T>;
        template<typename T>
        using EnableIfQueuePresentAndOutplace = typename std::enable_if<!std::is_same<T_Queue,DefaultQueue>::value && !T_InputWrapper::FFT_Def::isInplace,T>;
        template<typename T>
        using EnableIfNoQueueAndOutplace = typename std::enable_if<std::is_same<T_Queue,DefaultQueue>::value && !T_InputWrapper::FFT_Def::isInplace,T>;

        using FFT_Def = typename Input::FFT_Def;
        static_assert(std::is_same< FFT_Def, typename Output::FFT_Def>::value, "FFT types of input and output differs");
        using FFT_Properties = detail::FFT_Properties< FFT_Def, Input, Output, constructWithReadOnly >;
        using ActLibrary = typename bmpl::apply< Library, FFT_Properties >::type;
        static constexpr bool isInplace = FFT_Properties::isInplace;

        ActLibrary m_lib;
    public:

        explicit FFT(Input& input, Output& output)
        : m_lib(input, output)
        {
            static_assert(!isInplace, "Must not be called for inplace transforms");
        }

        explicit FFT(Input& inOut)
        : m_lib(inOut)
        {
            static_assert(isInplace, "Must not be called for out-of-place transforms");
        }

        template<typename T=int, typename EnableIfQueuePresentAndOutplace<T>::type=0 >
        explicit FFT(Input& input, Output& output, T_Queue& queue)
        : m_lib(input, output, queue)
        {
            static_assert(!isInplace, "Must not be called for inplace transforms");
        }

        template<typename T=int, typename EnableIfQueuePresentAndInplace<T>::type=0 >
        explicit FFT(Input& inOut, T_Queue& queue)
        : m_lib(inOut, queue)
        {
            static_assert(isInplace, "Must not be called for out-of-place transforms");
        }

        FFT(FFT&& obj)
        : m_lib(std::move(obj.m_lib))
        {}

        template<typename T=int, typename EnableIfNoQueueAndOutplace<T>::type=0 >
        void operator()(Input& input, Output& output)
        {
            static_assert(!isInplace, "Must not be called for inplace transforms");
            // Set full extents for C2R/R2C (rest is set in constructor)
            if(FFT_Def::kind == FFT_Kind::Complex2Real)
                input.setFullExtents(output.getExtents());
            else if(FFT_Def::kind == FFT_Kind::Real2Complex)
                output.setFullExtents(input.getExtents());
            input.preProcess();
            m_lib(input, output);
            output.postProcess();
        }

        template<typename T=int, typename EnableIfQueuePresentAndOutplace<T>::type=0 >
        void operator()(Input& input, Output& output, T_Queue& queue)
        {
            static_assert(!isInplace, "Must not be called for inplace transforms");
            // Set full extents for C2R/R2C (rest is set in constructor)
            if(FFT_Def::kind == FFT_Kind::Complex2Real)
                input.setFullExtents(output.getExtents());
            else if(FFT_Def::kind == FFT_Kind::Real2Complex)
                output.setFullExtents(input.getExtents());
            input.preProcess();
            m_lib(input, output, queue);
            output.postProcess();
        }

        template<typename T=int, typename EnableIfNoQueueAndInplace<T>::type=0 >
        void operator()(Input& inout)
        {
            static_assert(isInplace, "Must not be called for out-of-place transforms");
            inout.preProcess();
            m_lib(inout);
            inout.postProcess();
        }

        template<typename T=int, typename EnableIfQueuePresentAndInplace<T>::type=0 >
        void operator()(Input& inout, T_Queue& queue)
        {
            static_assert(isInplace, "Must not be called for out-of-place transforms");
            inout.preProcess();
            m_lib(inout, queue);
            inout.postProcess();
        }
    };

    template<
        class T_Library,
        bool T_constructWithReadOnly = true,
        typename T_InputWrapper,
        typename T_OutputWrapper
        >
    FFT< T_Library, T_InputWrapper, T_OutputWrapper, T_constructWithReadOnly >
    makeFFT(T_InputWrapper& input, T_OutputWrapper& output)
    {
        return FFT< T_Library, T_InputWrapper, T_OutputWrapper, T_constructWithReadOnly >(input, output);
    }

    template<
        class T_Library,
        bool T_constructWithReadOnly = true,
        typename T_DataWrapper
        >
    FFT< T_Library, T_DataWrapper, T_DataWrapper, T_constructWithReadOnly >
    makeFFT(T_DataWrapper& input)
    {
        return FFT< T_Library, T_DataWrapper, T_DataWrapper, T_constructWithReadOnly >(input);
    }

    template<
        class T_Library,
        typename T_Queue,
        bool T_constructWithReadOnly = true,
        typename T_InputWrapper,
        typename T_OutputWrapper
        >
    FFT< T_Library, T_InputWrapper, T_OutputWrapper, T_constructWithReadOnly, T_Queue >
    makeFFTInQueue(T_InputWrapper& input, T_OutputWrapper& output, T_Queue& queue)
    {
        return FFT< T_Library, T_InputWrapper, T_OutputWrapper, T_constructWithReadOnly, T_Queue >(input, output, queue);
    }

    template<
        class T_Library,
        typename T_Queue,
        bool T_constructWithReadOnly = true,
        typename T_DataWrapper
        >
    FFT< T_Library, T_DataWrapper, T_DataWrapper, T_constructWithReadOnly, T_Queue >
    makeFFTInQueue(T_DataWrapper& input, T_Queue& queue)
    {
        return FFT< T_Library, T_DataWrapper, T_DataWrapper, T_constructWithReadOnly, T_Queue >(input, queue);
    }


}  // namespace LiFFT
