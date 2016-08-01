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

#include "liFFT/traits/IntegralType.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/traits/IsAoS.hpp"
#include "liFFT/types/Vec.hpp"
#include "liFFT/accessors/DataContainerAccessor.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/policies/GetNumElements.hpp"
#include "liFFT/traits/IdentityAccessor.hpp"
#include "liFFT/traits/GetMemSize.hpp"
#include "liFFT/mem/RealValues.hpp"
#include "liFFT/mem/ComplexAoSValues.hpp"
#include "liFFT/void_t.hpp"
#include "liFFT/c++14_types.hpp"

#include <cassert>

namespace LiFFT {
    namespace mem {

        /**
         * Container used to store data with its meta-data
         */
        template< unsigned T_numDims, class T_Memory, class T_BaseAccessor = traits::IdentityAccessor_t<T_Memory>, bool T_isFlatMemory = true, bool T_isStrided=false >
        struct DataContainer
        {
            static constexpr unsigned numDims = T_numDims;
            using Memory = T_Memory;
            using BaseAccessor = T_BaseAccessor;
            static constexpr bool isStrided = T_isStrided;
            static constexpr bool isFlatMemory = T_isFlatMemory;

            using IdentityAccessor = accessors::DataContainerAccessor<T_isFlatMemory>;
            using IdxType = types::Vec< numDims, size_t >;

            friend struct accessors::DataContainerAccessor<isFlatMemory>;
            friend struct policies::GetExtents<DataContainer>;

            /**
             * Creates an uninitialized data container
             */
            DataContainer() = default;
            /**
             * Creates a data container with the specified extents and allocates the memory
             * Note: If the underlying memory does not support automatic destruction
             * you must use freeData() to free the memory
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents>
            DataContainer(T_Extents&& extents)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                allocData(extents);
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents>
            DataContainer(const Memory& data, T_Extents&& extents): m_data(data)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    m_extents[i] = extents[i];
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents>
            DataContainer(Memory&& data, T_Extents&& extents): m_data(std::move(data))
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    m_extents[i] = extents[i];
            }

            template< typename T_Idx >
            std::result_of_t< IdentityAccessor(T_Idx&, DataContainer&) >
            operator()(T_Idx&& idx)
            {
                assert(policies::checkSizes(idx, m_extents));
                return IdentityAccessor()(idx, *this);
            }

            template< typename T_Idx >
            std::result_of_t< IdentityAccessor(T_Idx&, const DataContainer&) >
            operator()(T_Idx&& idx) const
            {
                assert(policies::checkSizes(idx, m_extents));
                return IdentityAccessor()(idx, *this);
            }

            template<typename T_Extents>
            void
            setData(T_Extents&& extents, const Memory& data)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    m_extents[i] = extents[i];
                m_data = data;
            }

            template<typename T_Extents>
            void
            setData(T_Extents&& extents, Memory&& data)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    m_extents[i] = extents[i];
                m_data = std::move(data);
            }

            /**
              * Allocates data in the underlying memory if it supports that
              * Note: If the underlying memory does not support automatic destruction
              * you must use freeData() to free the memory
              */
             template<typename T_Extents>
             void
             allocData(T_Extents&& extents)
             {
                 static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                 for(unsigned i = 0; i < numDims; i++)
                     m_extents[i] = extents[i];
                 m_data.allocData(policies::getNumElements(*this, false));
             }

             size_t
             getMemSize() const
             {
                 return MemoryPolicy<Memory>::getMemSize(*this, m_data);
             }

             /**
               * Frees data in the underlying memory if it supports that
               */
             void
             freeData()
             {
                 m_data.freeData();
                 m_extents = IdxType::all(0);
             }

             template<class T>
             struct MemoryPolicy
             {
                 static auto
                 getData(T& data)
                 -> decltype(data.getData())
                 {
                     return data.getData();
                 }

                 static size_t
                 getMemSize(const DataContainer& /*container*/, const T& data)
                 {
                     return traits::getMemSize(data);
                 }
             };

             template<class T>
             struct MemoryPolicy<T*>
             {
                 static T*
                 getData(T* data)
                 {
                     return data;
                 }

                 static size_t
                 getMemSize(const DataContainer& container, const T* /*data*/)
                 {
                     return policies::getNumElements(container, false) * sizeof(T);
                 }
             };

             /**
              * Tries to return a pointer to the data
              * That is if Memory is a pointer type, data is returned, otherwise data.getData() is returned
              * @return Pointer to memory
              */
             std::result_of_t< decltype(&MemoryPolicy<Memory>::getData)(Memory&) >
             getData()
             {
                 return MemoryPolicy<Memory>::getData(m_data);
             }

             const IdxType&
             getExtents() const
             {
                 return m_extents;
             }

        protected:
             Memory m_data;
             IdxType m_extents;
        };

        template< unsigned T_numDims, class T_Memory, class T_BaseAccessor, bool T_isFlatMemory >
        struct DataContainer< T_numDims, T_Memory, T_BaseAccessor, T_isFlatMemory, true >: DataContainer< T_numDims, T_Memory, T_BaseAccessor, T_isFlatMemory, false >
        {
            using Parent = DataContainer< T_numDims, T_Memory, T_BaseAccessor, T_isFlatMemory, false >;

            using IdxType = typename Parent::IdxType;
            using Memory = typename Parent::Memory;
            static constexpr unsigned numDims = Parent::numDims;

            DataContainer(): Parent(){}
            /**
             * Creates a data container with the specified extents and strides and optionally allocates the memory
             * Note: If the underlying memory does not support automatic destruction
             * you must use freeData() to free the memory
             *
             * @param extents Extents of the container
             * @param strides Strides of the container
             */
            template<typename T_Extents, typename T_Strides>
            DataContainer(T_Extents&& extents, T_Strides&& strides): Parent(extents)
            {
                static_assert(traits::NumDims<T_Strides>::value >= numDims, "Wrong strides");
                for(unsigned i = 0; i < numDims; i++)
                    m_strides[i] = strides[i];
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents, typename T_Strides>
            DataContainer(const Memory& data, T_Extents&& extents, T_Strides&& strides): Parent(data, extents)
            {
                static_assert(traits::NumDims<T_Strides>::value >= numDims, "Wrong strides");
                for(unsigned i = 0; i < numDims; i++)
                    m_strides[i] = strides[i];
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents, typename T_Strides>
            DataContainer(Memory&& data, T_Extents&& extents, T_Strides&& strides): Parent(std::move(data), extents)
            {
                static_assert(traits::NumDims<T_Strides>::value >= numDims, "Wrong strides");
                for(unsigned i = 0; i < numDims; i++)
                    m_strides[i] = strides[i];
            }


        private:
            IdxType m_strides;
        };

        /**
         * A container storing real data with automatic memory management
         */
        template< unsigned T_numDims, typename T_Precision, bool T_isStrided = false >
        using RealContainer = DataContainer< T_numDims, RealValues<T_Precision>, traits::IdentityAccessor_t< RealValues<T_Precision> >, true, false >;

        /**
         * A container storing complex data with automatic memory management
         */
        template< unsigned T_numDims, typename T_Precision, bool T_isStrided = false >
        using ComplexContainer = DataContainer< T_numDims, ComplexAoSValues<T_Precision>, traits::IdentityAccessor_t< ComplexAoSValues<T_Precision> >, true, false >;

    }  // namespace mem

    namespace traits {

        template< class T >
        struct IntegralTypeImpl< T, void_t< typename T::Memory > >: IntegralType< typename T::Memory >{};

        template< class T >
        struct IsComplex<T, void_t< typename T::Memory > >: IsComplex< typename T::Memory >{};

        template< unsigned T_numDims, class T_Memory, class T_BaseAccessor, bool T_isFlatMemory >
        struct IsAoS< mem::DataContainer<T_numDims, T_Memory, T_BaseAccessor, T_isFlatMemory> >:
            IsAoS< typename mem::DataContainer<T_numDims, T_Memory, T_BaseAccessor, T_isFlatMemory>::Memory >{};

    }  // namespace traits

}  // namespace LiFFT
