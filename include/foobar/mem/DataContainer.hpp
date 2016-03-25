#pragma once

#include "foobar/traits/IntegralType.hpp"
#include "foobar/traits/IsComplex.hpp"
#include "foobar/traits/IsAoS.hpp"
#include "foobar/types/Vec.hpp"
#include "foobar/accessors/DataContainerAccessor.hpp"
#include "foobar/policies/GetExtents.hpp"
#include "foobar/policies/GetNumElements.hpp"
#include "foobar/traits/IdentityAccessor.hpp"
#include "foobar/traits/GetMemSize.hpp"
#include "foobar/mem/RealValues.hpp"
#include "foobar/mem/ComplexAoSValues.hpp"
#include "foobar/void_t.hpp"
#include "foobar/c++14_types.hpp"

#include <cassert>

namespace foobar {
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
            DataContainer(T_Extents&& rExtents)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                allocData(rExtents);
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents>
            DataContainer(const Memory& rData, T_Extents&& rExtents): data(rData)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    this->extents[i] = rExtents[i];
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents>
            DataContainer(Memory&& rData, T_Extents&& rExtents): data(std::move(rData))
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    this->extents[i] = rExtents[i];
            }

            template< typename T_Idx >
            std::result_of_t< IdentityAccessor(T_Idx&, DataContainer&) >
            operator()(T_Idx&& idx)
            {
                assert(policies::checkSizes(idx, extents));
                return IdentityAccessor()(idx, *this);
            }

            template< typename T_Idx >
            std::result_of_t< IdentityAccessor(T_Idx&, const DataContainer&) >
            operator()(T_Idx&& idx) const
            {
                assert(policies::checkSizes(idx, extents));
                return IdentityAccessor()(idx, *this);
            }

            template<typename T_Extents>
            void
            setData(T_Extents&& rExtents, const Memory& rData)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    this->extents[i] = rExtents[i];
                this->data = rData;
            }

            template<typename T_Extents>
            void
            setData(T_Extents&& rExtents, Memory&& rData)
            {
                static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                for(unsigned i = 0; i < numDims; i++)
                    this->extents[i] = rExtents[i];
                this->data = std::move(rData);
            }

            /**
              * Allocates data in the underlying memory if it supports that
              * Note: If the underlying memory does not support automatic destruction
              * you must use freeData() to free the memory
              */
             template<typename T_Extents>
             void
             allocData(T_Extents&& rExtents)
             {
                 static_assert(traits::NumDims<T_Extents>::value >= numDims, "Wrong extents");
                 for(unsigned i = 0; i < numDims; i++)
                     this->extents[i] = rExtents[i];
                 data.allocData(policies::getNumElements(*this, false));
             }

             size_t
             getMemSize() const
             {
                 return MemoryPolicy<Memory>::getMemSize(*this, data);
             }

             /**
               * Frees data in the underlying memory if it supports that
               */
             void
             freeData()
             {
                 data.freeData();
                 extents = IdxType::all(0);
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
                 getMemSize(const DataContainer& container, const T& data)
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
                 getMemSize(const DataContainer& container, const T* data)
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
                 return MemoryPolicy<Memory>::getData(data);
             }

             const IdxType&
             getExtents() const
             {
                 return extents;
             }

        protected:
             Memory data;
             IdxType extents;
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
            DataContainer(T_Extents&& rExtents, T_Strides&& rStrides): Parent(rExtents)
            {
                static_assert(traits::NumDims<T_Strides>::value >= numDims, "Wrong strides");
                for(unsigned i = 0; i < numDims; i++)
                    this->strides[i] = rStrides[i];
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents, typename T_Strides>
            DataContainer(const Memory& rData, T_Extents&& rExtents, T_Strides&& rStrides)
            : Parent(rData, rExtents)
            {
                static_assert(traits::NumDims<T_Strides>::value >= numDims, "Wrong strides");
                for(unsigned i = 0; i < numDims; i++)
                    this->strides[i] = rStrides[i];
            }

            /**
             * Creates a data container with the specified extents
             *
             * @param extents Extents of the container
             */
            template<typename T_Extents, typename T_Strides>
            DataContainer(Memory&& rData, T_Extents&& extents, T_Strides&&): Parent(std::move(rData), extents)
            {
                static_assert(traits::NumDims<T_Strides>::value >= numDims, "Wrong strides");
                for(unsigned i = 0; i < numDims; i++)
                    this->strides[i] = strides[i];
            }


        private:
            IdxType strides;
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

        template< class T >
        struct IsAoS<T, void_t< typename T::Memory > >: IsAoS< typename T::Memory >{};

    }  // namespace traits

}  // namespace foobar
