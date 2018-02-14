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

#include "libLiFFT/AutoDetect.hpp"
#include "libLiFFT/FFT_DataWrapper.hpp"
#include "libLiFFT/traits/NumDims.hpp"
#include "libLiFFT/traits/IsComplex.hpp"
#include "libLiFFT/traits/IsAoS.hpp"
#include "libLiFFT/traits/IsStrided.hpp"
#include "libLiFFT/policies/GetExtents.hpp"
#include "libLiFFT/policies/GetStrides.hpp"
#include "libLiFFT/mem/RealValues.hpp"
#include "libLiFFT/mem/ComplexAoSValues.hpp"
#include "libLiFFT/mem/ComplexSoAValues.hpp"
#include "libLiFFT/mem/DataContainer.hpp"
#include "libLiFFT/traits/IdentityAccessor.hpp"
#include "libLiFFT/traits/IsDeviceMemory.hpp"
#include "libLiFFT/types/SymmetricWrapper.hpp"
#include "libLiFFT/c++14_types.hpp"
#include <type_traits>
#include <functional>

namespace LiFFT {

    // Fwd decl
    template< class T1, class T2, class T3, bool t, class T4>
    class FFT;

    namespace detail {

        template< bool T_isInplace = false >
        class FFT_LibPtrWrapperBase
        {
        protected:
            void preProcess(){}
            void postProcess(){}
            FFT_LibPtrWrapperBase(){}
            ~FFT_LibPtrWrapperBase(){}
        };

        template<>
        class FFT_LibPtrWrapperBase<true>
        {
        protected:
            void preProcess(){
                if(m_inplaceOutput)
                    m_inplaceOutput->preProcess();
            }
            void postProcess(){
                if(m_inplaceOutput)
                    m_inplaceOutput->postProcess();
            }
            detail::IInplaceOutput* m_inplaceOutput;

            FFT_LibPtrWrapperBase(): m_inplaceOutput(nullptr){}
            ~FFT_LibPtrWrapperBase(){}
        };

    }  // namespace detail

    /**
     * Wrapper for the data
     */
    template<
        class T_FFT_Def,
        class T_IsInput,
        typename T_LibPtr,
//        class T_HasInstance = std::false_type,
//        typename T_BaseAccessor = traits::IdentityAccessor_t<T_Base>,
        unsigned numDims,
        bool T_isComplex,
        typename PrecisionType
    >
    class FFT_LibPtrWrapper: public detail::FFT_LibPtrWrapperBase<T_FFT_Def::isInplace>
    {
    public:
        static constexpr bool isAoS = true;
        static constexpr bool isStrided = false;
        static constexpr bool isComplex = T_isComplex;

        using FFT_Def =  T_FFT_Def;
        static constexpr bool isInput = T_IsInput::value;
        //using Base = T_Base;
        //using BaseAccessor = T_BaseAccessor;
        //static constexpr bool hasInstance = T_HasInstance::value;

        using Parent = detail::FFT_LibPtrWrapperBase<T_FFT_Def::isInplace>;

        static_assert(!FFT_Def::isInplace || isInput, "Only the input container must be specified for inplace transforms");
        static_assert(numDims == FFT_Def::numDims, "Wrong number of dimensions");
        // using IdxType = types::Vec<numDims>;
        // using AccRefType = std::result_of_t<BaseAccessor(const IdxType&, Base&)>;
        // using AccType = typename std::remove_reference<AccRefType>::type;
        //static constexpr bool isComplex = traits::IsComplex< AccType >::value;
        static_assert( (isInput && (isComplex || !FFT_Def::isComplexInput)) ||
                       (!isInput && (isComplex || !FFT_Def::isComplexOutput)),
                       "Wrong element type for this FFT: Expected complex, got real" );
        static_assert( (isInput && (!isComplex || FFT_Def::isComplexInput)) ||
                       (!isInput && (!isComplex || FFT_Def::isComplexOutput)),
                       "Wrong element type for this FFT: Expected real, got complex" );

        using Extents = types::Vec<numDims>;
        // Precision (double, float...) is the base type of RawPtrType (which is a pointer)
        // For Complex-SoA values RawPtrType is a std::pair of pointers
        //using PrecisionType = typename traits::IntegralType<AccType>::type;
        static_assert(std::is_same<PrecisionType, typename FFT_Def::PrecisionType>::value, "Wrong precision type");

        //static constexpr bool needOwnMemoryPtr = !std::is_reference<AccRefType>::value || std::is_const<AccRefType>::value;
        //static_assert(!FFT_Def::isInplace || !needOwnMemoryPtr, "Memory used is not writable");
        //static constexpr bool isAoS = needOwnMemoryPtr || traits::IsAoS< Base >::value;
        //static constexpr bool isStrided = !needOwnMemoryPtr && traits::IsStrided< Base >::value;
        using IsDeviceMemory = std::true_type;//traits::IsDeviceMemory< Base >;

        // using IdentityAccessor = accessors::ArrayAccessor<true>;

        // using RefType = typename std::add_lvalue_reference<Base>::type;
        // using InstanceType = std::conditional_t< hasInstance, Base, RefType >;
        // using ParamType = typename std::conditional_t< hasInstance, std::add_rvalue_reference<Base>, std::add_lvalue_reference<Base> >::type;
        // static constexpr bool isHalfData = (FFT_Def::kind == FFT_Kind::Complex2Real && isInput) ||
        //                                    (FFT_Def::kind == FFT_Kind::Real2Complex && !isInput);

    private:
        // InstanceType m_base;
        // BaseAccessor m_acc;
        // Memory m_memory;
        T_LibPtr m_libptr;
        Extents m_extents, m_fullExtents;
        // std::unique_ptr<MemoryFallback> m_memFallback;

        void
        setFullExtents(const Extents& extents)
        {
            m_fullExtents = extents;
        }
        template< class T1, class T2, class T3, bool t, class T4>
        friend class FFT;
    public:
        template<typename T_Extents>
        FFT_LibPtrWrapper(T_LibPtr libptr, T_Extents&& extents):
            m_libptr(libptr)
        {
            static_assert(!FFT_Def::isInplace || FFT_Def::kind != FFT_Kind::Complex2Real, "No real extents set");

            for(unsigned i=0; i<numDims; ++i)
                m_extents[i] = extents[i];

            // Set full extents for real data or for C2C transforms
            // The others are set to 0 here and updated during FFT execution
            if(FFT_Def::kind == FFT_Kind::Complex2Complex ||
                    (FFT_Def::kind == FFT_Kind::Complex2Real && !isInput) ||
                    (FFT_Def::kind == FFT_Kind::Real2Complex && isInput) )
                m_fullExtents = m_extents;
            else
                m_fullExtents = m_fullExtents.all(0);
        }

        template<typename T_Extents>
        FFT_LibPtrWrapper(T_LibPtr libptr, unsigned fullSizeLastDim, T_Extents&& extents):
            m_libptr(libptr)
        {
            for(unsigned i=0; i<numDims; ++i)
                m_extents[i] = extents[i];
            if((FFT_Def::kind == FFT_Kind::Complex2Real && isInput) ||
                    (FFT_Def::kind == FFT_Kind::Real2Complex && !isInput) ||
                    FFT_Def::kind == FFT_Kind::Complex2Complex)
            {
                if(m_extents[numDims - 1] != fullSizeLastDim)
                    throw std::runtime_error("Invalid size given");
            }else if((FFT_Def::kind == FFT_Kind::Complex2Real && !isInput) ||
                    (FFT_Def::kind == FFT_Kind::Real2Complex && isInput))
            {
                if(m_extents[numDims - 1] != fullSizeLastDim / 2 + 1)
                    throw std::runtime_error("Invalid size given");
            }
            if(FFT_Def::isInplace)
            {
                m_fullExtents = m_extents;
                m_fullExtents[numDims - 1] = fullSizeLastDim;
            }
        }

        /**
         * Returns a reference to the base class
         * @return Reference to base data
         */
        T_LibPtr
        getBase()
        {
            return m_libptr;
        }

        /**
         * Returns the pointer to the internal data
         * This might be memory allocated by this class or the memory from the base type
         *
         * @return Complex* or Real*
         */
        T_LibPtr
        getDataPtr()
        {
            return m_libptr;
        }

        size_t getMemSize() const
        {
            return sizeof(PrecisionType)*getNumElements();
        }

        /**
         * Returns the extents object
         * Supports iterators
         *
         * @return Extents object
         */
        const Extents&
        getExtents() const
        {
            return m_extents;
        }

        /**
         * Returns the full extents object
         * Supports iterators
         *
         * @return Extents object
         */
        const Extents&
        getFullExtents() const
        {
            return m_fullExtents;
        }

        /**
         * Returns the number of actual elements (ignoring strides)
         */
        size_t
        getNumElements() const
        {
            return policies::getNumElementsFromExtents(m_extents);
        }

        /**
         * Internal method. Called before each FFT for input
         * Copies data to internal memory if required
         */
        void
        preProcess()
        {
            Parent::preProcess();
        }

        /**
         * Internal method. Called after each FFT for output wrappers
         * Copies data from internal memory if required
         */
        void
        postProcess()
        {
            Parent::postProcess();
        }
    };

    template< class T_FFT_Def, // FFT_Definition (FFT attributes)
              typename T_LibPtr,
              unsigned numDims = 2,
              bool T_isComplex = true,
              typename PrecisionType = float
              >
    using FFT_InputLibPtrWrapper = FFT_LibPtrWrapper< T_FFT_Def,
                                                      std::true_type,
                                                      T_LibPtr,
                                                      numDims,
                                                      T_isComplex,
                                                      PrecisionType>;

    template< class T_FFT_Def, // FFT_Definition (FFT attributes)
              typename T_LibPtr,
              unsigned numDims = 2,
              bool T_isComplex = true,
              typename PrecisionType = float
              >
    using FFT_OutputLibPtrWrapper = FFT_LibPtrWrapper< T_FFT_Def,
                                                       std::false_type,
                                                       T_LibPtr,
                                                       numDims,
                                                       T_isComplex,
                                                       PrecisionType>;
}  // namespace LiFFT
