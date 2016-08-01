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

#include "liFFT/FFT_Properties.hpp"

// Included for convenience, so only one include is required from user code
#include "liFFT/FFT_Definition.hpp"
#include "liFFT/FFT_DataWrapper.hpp"
#include <boost/mpl/apply.hpp>

namespace bmpl = boost::mpl;

namespace LiFFT {

    template< class T_Input, class T_Output >
    class FFT_Interface_Inplace
    {
    public:
        virtual ~FFT_Interface_Inplace(){};

        virtual void operator()(T_Input& inout) = 0;
    };

    template< class T_Input, class T_Output >
    class FFT_Interface_Outplace
    {
    public:
        virtual ~FFT_Interface_Outplace(){};

        virtual void operator()(T_Input& input, T_Output& output) = 0;
    };

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
            bool T_constructWithReadOnly = true
            >
    class FFT:
            public std::conditional_t<
                T_InputWrapper::FFT_Def::isInplace,
                FFT_Interface_Inplace< T_InputWrapper, T_InputWrapper >,
                FFT_Interface_Outplace< T_InputWrapper, T_OutputWrapper >
            >
    {
        using Library = T_Library;
        using Input = T_InputWrapper;
        using Output = T_OutputWrapper;
        static constexpr bool constructWithReadOnly = T_constructWithReadOnly;

        using FFT_Def = typename Input::FFT_Def;
        static_assert(std::is_same< FFT_Def, typename Output::FFT_Def>::value, "FFT types of input and output differs");
        using FFT_Properties = detail::FFT_Properties< FFT_Def, Input, Output, constructWithReadOnly >;
        using ActLibrary = typename bmpl::apply< Library, FFT_Properties >::type;
        static constexpr bool isInplace = FFT_Properties::isInplace;

        ActLibrary m_lib;
    public:
        explicit FFT(Input& input, Output& output): m_lib(input, output)
        {
            static_assert(!isInplace, "Must not be called for inplace transforms");
        }

        explicit FFT(Input& inOut): m_lib(inOut)
        {
            static_assert(isInplace, "Must not be called for out-of-place transforms");
        }

        FFT(FFT&& obj): m_lib(std::move(obj.m_lib)){}

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

        void operator()(Input& inout)
        {
            static_assert(isInplace, "Must not be called for out-of-place transforms");
            inout.preProcess();
            m_lib(inout);
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

}  // namespace LiFFT
