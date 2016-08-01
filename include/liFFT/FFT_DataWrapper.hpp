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

#include "liFFT/AutoDetect.hpp"
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/traits/IsAoS.hpp"
#include "liFFT/traits/IsStrided.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/policies/GetStrides.hpp"
#include "liFFT/mem/RealValues.hpp"
#include "liFFT/mem/ComplexAoSValues.hpp"
#include "liFFT/mem/ComplexSoAValues.hpp"
#include "liFFT/mem/DataContainer.hpp"
#include "liFFT/traits/IdentityAccessor.hpp"
#include "liFFT/traits/IsDeviceMemory.hpp"
#include "liFFT/types/SymmetricWrapper.hpp"
#include "liFFT/c++14_types.hpp"
#include "liFFT/FFT_Memory.hpp"
#include <type_traits>
#include <functional>

namespace LiFFT {

    // Fwd decl
    template< class T1, class T2, class T3, bool t>
    class FFT;

    namespace detail {

        template< class T_FFT_DataWrapper, bool T_isHalfData = T_FFT_DataWrapper::isHalfData >
        struct GetFullData
        {
            types::SymmetricWrapper<T_FFT_DataWrapper>
            operator()(T_FFT_DataWrapper& data) const
            {
                return types::makeSymmetricWrapper(data, data.getFullExtents()[data.numDims-1]);
            }
        };

        template<class T_FFT_DataWrapper>
        struct GetFullData< T_FFT_DataWrapper, false >
        {
            T_FFT_DataWrapper
            operator()(T_FFT_DataWrapper& data) const
            {
                return data;
            }
        };

        struct IInplaceOutput
        {
            virtual void preProcess() = 0;
            virtual void postProcess() = 0;
        protected:
            IInplaceOutput(){}
            virtual ~IInplaceOutput(){}
        };

        template< bool T_isInplace = false >
        class FFT_DataWrapperBase
        {
        protected:
            void preProcess(){}
            void postProcess(){}
            FFT_DataWrapperBase(){}
            ~FFT_DataWrapperBase(){}
        };

        template<>
        class FFT_DataWrapperBase<true>
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

            FFT_DataWrapperBase(): m_inplaceOutput(nullptr){}
            ~FFT_DataWrapperBase(){}
        };

    }  // namespace detail

    /**
     * Gets the complete dataset for a dataset of a C2R/R2C FFT
     * Returns the same data for the real types or any wrapper used in a C2C FFT
     * MUST be called AFTER the FFT was executed!
     *
     * @param data FFT_DataWrapper instance
     * @return Complete dataset
     */
    template< class T_FFT_DataWrapper >
    std::result_of_t<detail::GetFullData<T_FFT_DataWrapper>(T_FFT_DataWrapper&)>
    getFullData(T_FFT_DataWrapper& data)
    {
        return detail::GetFullData<T_FFT_DataWrapper>()(data);
    }

    /**
     * Wrapper for the data
     */
    template<
        class T_FFT_Def,
        class T_IsInput,
        typename T_Base,
        class T_HasInstance = std::false_type,
        typename T_BaseAccessor = traits::IdentityAccessor_t<T_Base>
    >
    class FFT_DataWrapper: public detail::FFT_DataWrapperBase<T_FFT_Def::isInplace>
    {
    public:
        using FFT_Def =  T_FFT_Def;
        static constexpr bool isInput = T_IsInput::value;
        using Base = T_Base;
        using BaseAccessor = T_BaseAccessor;
        static constexpr bool hasInstance = T_HasInstance::value;

        using Parent = detail::FFT_DataWrapperBase<T_FFT_Def::isInplace>;

        static_assert(!FFT_Def::isInplace || isInput, "Only the input container must be specified for inplace transforms");
        static constexpr unsigned numDims = traits::NumDims< Base >::value;
        static_assert(numDims == FFT_Def::numDims, "Wrong number of dimensions");
        using IdxType = types::Vec<numDims>;
        using AccRefType = std::result_of_t<BaseAccessor(const IdxType&, Base&)>;
        using AccType = typename std::remove_reference<AccRefType>::type;
        static constexpr bool isComplex = traits::IsComplex< AccType >::value;
        static_assert( (isInput && (isComplex || !FFT_Def::isComplexInput)) ||
                       (!isInput && (isComplex || !FFT_Def::isComplexOutput)),
                       "Wrong element type for this FFT: Expected complex, got real" );
        static_assert( (isInput && (!isComplex || FFT_Def::isComplexInput)) ||
                       (!isInput && (!isComplex || FFT_Def::isComplexOutput)),
                       "Wrong element type for this FFT: Expected real, got complex" );

        using Extents = IdxType;
        // Precision (double, float...) is the base type of RawPtrType (which is a pointer)
        // For Complex-SoA values RawPtrType is a std::pair of pointers
        using PrecisionType = typename traits::IntegralType<AccType>::type;
        static_assert(std::is_same<PrecisionType, typename FFT_Def::PrecisionType>::value, "Wrong precision type");

        static constexpr bool needOwnMemoryPtr = !std::is_reference<AccRefType>::value || std::is_const<AccRefType>::value;
        static_assert(!FFT_Def::isInplace || !needOwnMemoryPtr, "Memory used is not writable");
        static constexpr bool isAoS = needOwnMemoryPtr || traits::IsAoS< Base >::value;
        static constexpr bool isStrided = !needOwnMemoryPtr && traits::IsStrided< Base >::value;
        using IsDeviceMemory = traits::IsDeviceMemory< Base >;

        using Memory_t = mem::DataContainer<
                             numDims,
                             std::conditional_t<
                                 isComplex,
                                 std::conditional_t<
                                     isAoS,
                                     mem::ComplexAoSValues<PrecisionType, needOwnMemoryPtr>,
                                     mem::ComplexSoAValues<PrecisionType, needOwnMemoryPtr>
                                 >,
                                 mem::RealValues<PrecisionType, needOwnMemoryPtr>
                             >
                         >;
        using MemoryFallback_t = mem::DataContainer<
                             numDims,
                             std::conditional_t<
                                 isComplex,
                                 std::conditional_t<
                                     isAoS,
                                     mem::ComplexAoSValues<PrecisionType>,
                                     mem::ComplexSoAValues<PrecisionType>
                                 >,
                                 mem::RealValues<PrecisionType>
                             >
                         >;
        using Memory = detail::FFT_Memory< Memory_t, needOwnMemoryPtr >;
        using MemoryFallback = detail::FFT_Memory< MemoryFallback_t, true >;

        using IdentityAccessor = accessors::ArrayAccessor<true>;

        using RefType = typename std::add_lvalue_reference<Base>::type;
        using InstanceType = std::conditional_t< hasInstance, Base, RefType >;
        using ParamType = typename std::conditional_t< hasInstance, std::add_rvalue_reference<Base>, std::add_lvalue_reference<Base> >::type;
        static constexpr bool isHalfData = (FFT_Def::kind == FFT_Kind::Complex2Real && isInput) ||
                                           (FFT_Def::kind == FFT_Kind::Real2Complex && !isInput);

    private:
        InstanceType m_base;
        BaseAccessor m_acc;
        Extents m_extents, m_fullExtents;
        Memory m_memory;
        std::unique_ptr<MemoryFallback> m_memFallback;

        void
        setFullExtents(const Extents& extents)
        {
            m_fullExtents = extents;
        }
        template< class T1, class T2, class T3, bool t>
        friend class FFT;
    public:
        FFT_DataWrapper(ParamType data, BaseAccessor acc = BaseAccessor()):
            m_base(static_cast<ParamType>(data)), m_acc(std::move(acc))
        {
            static_assert(!FFT_Def::isInplace || FFT_Def::kind != FFT_Kind::Complex2Real, "No real extents set");

            policies::GetExtents<Base> extents(m_base);
            for(unsigned i=0; i<numDims; ++i)
                m_extents[i] = extents[i];
            m_memory.init(m_extents);
            // Set full extents for real data or for C2C transforms
            // The others are set to 0 here and updated during FFT execution
            if(FFT_Def::kind == FFT_Kind::Complex2Complex ||
                    (FFT_Def::kind == FFT_Kind::Complex2Real && !isInput) ||
                    (FFT_Def::kind == FFT_Kind::Real2Complex && isInput) )
                m_fullExtents = m_extents;
            else
                m_fullExtents = m_fullExtents.all(0);
            if(m_memory.checkPtr(m_base, m_acc, FFT_Def::isInplace && !isComplex))
                m_memFallback = nullptr;
            else if(FFT_Def::isInplace)
                throw std::runtime_error("Cannot use given memory as the strides/indexing is wrong!");
            else
            {
                m_memFallback.reset(new MemoryFallback());
                m_memFallback->init(m_extents);
            }
        }

        FFT_DataWrapper(ParamType data, unsigned fullSizeLastDim, BaseAccessor acc = BaseAccessor()):
            m_base(static_cast<ParamType>(data)), m_acc(std::move(acc))
        {
            policies::GetExtents<Base> extents(m_base);
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

            m_memory.init(m_extents);
            if(m_memory.checkPtr(m_base, m_acc, FFT_Def::isInplace && !isComplex))
                m_memFallback = nullptr;
            else if(FFT_Def::isInplace)
                throw std::runtime_error("Cannot use given memory as the strides/indexing is wrong!");
            else
            {
                m_memFallback.reset(new MemoryFallback());
                m_memFallback->init(m_extents);
            }
        }

        /**
         * Returns an element from the base class
         * @param idx Index
         * @return    Element
         */
        std::result_of_t< BaseAccessor(const IdxType&, Base&) >
        operator()(const IdxType& idx)
        {
            return m_acc(idx, m_base);
        }

        /**
         * Returns an element from the base class
         * @param idx Index
         * @return    Element
         */
        std::result_of_t< BaseAccessor(const IdxType&, const Base&) >
        operator()(const IdxType& idx) const
        {
            return m_acc(idx, const_cast<const Base&>(m_base));
        }

        /**
         * Returns a reference to the base class
         * @return Reference to base data
         */
        RefType
        getBase()
        {
            return m_base;
        }

        /**
         * Returns the pointer to the internal data
         * This might be memory allocated by this class or the memory from the base type
         *
         * @return Complex* or Real*
         */
        auto
        getDataPtr()
        -> decltype(m_memory.getPtr(m_base, m_acc))
        {
            if(m_memFallback)
                return m_memFallback->getPtr(m_base, m_acc);
            else
                return m_memory.getPtr(m_base, m_acc);
        }

        size_t getMemSize() const
        {
            if(FFT_Def::isInplace)
                return traits::getMemSize(m_base);
            else if(m_memFallback)
                return traits::getMemSize(*m_memFallback);
            else
                return traits::getMemSize(m_memory);
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
            if(isInput)
            {
                if(m_memFallback)
                    m_memFallback->copyFrom(m_base, m_acc);
                else
                    m_memory.copyFrom(m_base, m_acc);
            }
            Parent::preProcess();
        }

        /**
         * Internal method. Called after each FFT for output wrappers
         * Copies data from internal memory if required
         */
        void
        postProcess()
        {
            if(!isInput)
            {
                if(m_memFallback)
                    m_memFallback->copyTo(m_base, m_acc);
                else
                    m_memory.copyTo(m_base, m_acc);
            }
            Parent::postProcess();
        }
    };

    template< class T_FFT_Def, typename T_Base, class T_HasInstance = std::false_type, typename T_BaseAccessor = traits::IdentityAccessor_t<T_Base> >
    using FFT_InputDataWrapper = FFT_DataWrapper< T_FFT_Def, std::true_type, T_Base, T_HasInstance, T_BaseAccessor >;

    template< class T_FFT_Def, typename T_Base, class T_HasInstance = std::false_type, typename T_BaseAccessor = traits::IdentityAccessor_t<T_Base> >
    using FFT_OutputDataWrapper = FFT_DataWrapper< T_FFT_Def, std::false_type, T_Base, T_HasInstance, T_BaseAccessor >;

}  // namespace LiFFT
