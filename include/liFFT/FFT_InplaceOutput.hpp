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

#include "liFFT/accessors/ArrayAccessor.hpp"
#include "liFFT/FFT_DataWrapper.hpp"
#include "liFFT/c++14_types.hpp"
#include "liFFT/traits/IdentityAccessor.hpp"
#include "liFFT/types/Real.hpp"
#include "liFFT/types/Complex.hpp"
#include "liFFT/mem/PlainPtrWrapper.hpp"
#include <stdexcept>

namespace LiFFT {

    namespace detail {
        namespace traits {
            template< typename T_Precision, FFT_Kind T_kind = FFT_Kind::Complex2Complex >
            struct AllowedOutputData
            {
                using Input  = types::Complex<T_Precision>*;
                using Output = types::Complex<T_Precision>*;
            };
            template< typename T_Precision >
            struct AllowedOutputData< T_Precision, FFT_Kind::Complex2Real >
            {
                using Input  = types::Complex<T_Precision>*;
                using Output = types::Real<T_Precision>*;
            };
            template< typename T_Precision >
            struct AllowedOutputData< T_Precision, FFT_Kind::Real2Complex >
            {
                using Input  = types::Real<T_Precision>*;
                using Output = types::Complex<T_Precision>*;
            };
        }  // namespace traits
    }  // namespace detail

    /**
     * Class used to access the output of an inplace FFT
     * \tparam T_FFT_Input InputDataWrapper
     */
    template< class T_FFT_Input >
    class FFT_InplaceOutput: public detail::IInplaceOutput
    {
        using Input = T_FFT_Input;
        using FFT_Def = typename Input::FFT_Def;
        using InputAccessor = typename Input::IdentityAccessor;

    public:
        static constexpr unsigned numDims = Input::numDims;
        static constexpr bool isComplex   = FFT_Def::isComplexOutput;
        static constexpr bool isAoS       = Input::isAoS;
        static constexpr bool isStrided   = Input::isStrided;
        static constexpr bool isHalfData = FFT_Def::kind == FFT_Kind::Real2Complex;

        static_assert(isAoS, "Inplace FFTs must use Array of Structs");

        using IdxType = typename Input::IdxType;
        using Extents = typename Input::Extents;
        using PrecisionType = typename Input::PrecisionType;

    private:
        using Value = std::conditional_t<
                            isComplex,
                            types::Complex<PrecisionType>,
                            types::Real<PrecisionType>
                        >;
        using Data = mem::PlainPtrWrapper<std::integral_constant<unsigned, numDims>, Value, std::integral_constant<bool, isStrided>, typename Input::IsDeviceMemory>;

    public:
        using IdentityAccessor = accessors::ArrayAccessor<true>;

        FFT_InplaceOutput(Input& input): m_input(input), m_data(convertPtr(nullptr), IdxType::all(0u))
        {
            updateData();
        }

        std::result_of_t< Data(const IdxType&) >
        operator()(const IdxType& idx)
        {
            return m_data(idx);
        }

        std::result_of_t< const Data(const IdxType&) >
        operator()(const IdxType& idx) const
        {
            return m_data(idx);
        }

        const typename Data::IdxType&
        getExtents() const
        {
            return m_data.getExtents();
        }

        const Extents&
        getFullExtents() const
        {
            return m_fullExtents;
        }

        void preProcess(){}
        void postProcess(){
            updateData();
        }
    private:
        Input& m_input;
        Data m_data;
        Extents m_fullExtents;

        void updateData(){
            Extents extents;
            switch (FFT_Def::kind) {
                case FFT_Kind::Complex2Complex:
                    extents = m_input.getExtents();
                    m_fullExtents = extents;
                    break;
                case FFT_Kind::Real2Complex:
                    extents = m_input.getExtents();
                    m_fullExtents = extents;
                    extents[numDims - 1] = extents[numDims - 1] / 2 + 1;
                    break;
                case FFT_Kind::Complex2Real:
                    extents = m_input.getFullExtents();
                    m_fullExtents = extents;
                    break;
                default:
                    throw std::logic_error("Wrong FFT kind!");
            }
            size_t numElements = policies::getNumElementsFromExtents(extents);
            if(traits::getMemSize(m_input) < numElements * sizeof(Value))
                throw std::runtime_error("Number of elements is wrong or not enough memory allocated");
            m_data = Data(convertPtr(m_input.getDataPtr()), extents);
        }

        using AllowedOutputData = detail::traits::AllowedOutputData< PrecisionType, FFT_Def::kind >;
        static typename AllowedOutputData::Output
        convertPtr(typename AllowedOutputData::Input ptr)
        {
            return reinterpret_cast<typename AllowedOutputData::Output>(ptr);
        }
    };

}  // namespace LiFFT
