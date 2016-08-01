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

#include "liFFT/policies/Copy.hpp"
#include "liFFT/policies/SafePtrCast.hpp"
#include "liFFT/traits/NumDims.hpp"
#include "liFFT/traits/GetMemSize.hpp"
#include "liFFT/policies/GetExtents.hpp"
#include "liFFT/accessors/ConvertAccessor.hpp"
#include <algorithm>

namespace LiFFT {
namespace detail {

    /**
     * Policy for managing the memory of the FFT_DataWrapper
     *
     * \tparam T_Memory Type of the memory to manage (RealValues/Complex*Values)
     * \tparam T_needOwnMemory True if memory should be allocated or if the base memory can be used
     */
    template< typename T_Memory, bool T_needOwnMemory = true >
    struct FFT_Memory
    {
        using Memory = T_Memory;
        using Ptr = std::result_of_t< decltype(&Memory::Memory::getData)(typename Memory::Memory) >;
        static constexpr unsigned numDims = traits::NumDims< Memory >::value;

        using MemAcc = traits::IdentityAccessor_t< Memory >;
        using DataType = std::remove_reference_t< std::result_of_t< MemAcc(types::Vec<numDims>&, Memory&) > >;

        Memory m_data;

        /**
         * Allocates memory such that a matrix with the given extents fits in
         *
         * @param extents Extents of the memory. Needs to support std::begin/end
         */
        template< typename T_Extents >
        void
        init(const T_Extents& extents)
        {
            m_data.allocData(extents);
        }

        /**
         * Returns a pointer to the start of the data
         *
         * @param unused
         * @param unused
         * @return Pointer to the data
         */
        template< class T_Obj, class T_Acc >
        Ptr
        getPtr(T_Obj&, T_Acc&)
        {
            return m_data.getData();
        }

        /**
         * Copies data from the given object into this memory
         *
         * @param obj Object to read from
         * @param acc Accessor to access the elements
         */
        template< class T_Obj, class T_Acc >
        void
        copyFrom(T_Obj& obj, T_Acc&& acc)
        {
            auto copy = policies::makeCopy(std::forward<T_Acc>(acc), MemAcc());
            copy(obj, m_data);
        }

        /**
         * Copies data from this memory to the given object
         *
         * @param obj Object to write to
         * @param acc Accessor to access the elements
         */
        template< class T_Obj, class T_Acc >
        void
        copyTo(T_Obj& obj, T_Acc&& acc) const
        {
            using ObjDataType = std::remove_reference_t< std::result_of_t< T_Acc(types::Vec<numDims>&, T_Obj&) > >;
            using PlainAcc = std::remove_cv_t< std::remove_reference_t< T_Acc > >;
            using AccSrc = std::conditional_t<
                    traits::IsBinaryCompatible<DataType, ObjDataType>::value &&
                        !std::is_same<DataType, ObjDataType>::value,
                    accessors::ConvertAccessor<PlainAcc, DataType>,
                    PlainAcc>;

            auto copy = policies::makeCopy(MemAcc(), AccSrc(std::forward<T_Acc>(acc)));
            copy(m_data, obj);
        }

        /**
         * Checks whether the pointer(s) is/are valid for use. That is they point to contiguous memory
         * @return
         */
        template< class T_Obj, class T_Acc >
        bool checkPtr(const T_Obj&, const T_Acc&, bool) const {
            return true;
        }

        size_t
        getMemSize() const
        {
            return traits::getMemSize(m_data);
        }
    };

    template< typename T_Ptr, bool T_isPlainPtr = std::is_pointer<T_Ptr>::value >
    struct FFT_Memory_GetPtr
    {
        using Ptr = T_Ptr;
    private:
        template< class T_Obj, class T_Acc, class T_Idx >
        Ptr
        doGetPtr(T_Obj& obj, T_Acc& acc, T_Idx& idx) const
        {
            return policies::safe_ptr_cast<Ptr>(&acc(idx, obj));
        }
    public:

        /**
         * Returns a pointer to the start of the data in the given object
         * Enabled if we should return a plain pointer
         *
         * @param obj Object where the data is stored
         * @param acc Accessor to access the elements in @ref obj
         * @return Pointer to the element at acc({0, ... } obj)
         */
        template< class T_Obj, class T_Acc >
        Ptr
        getPtr(T_Obj& obj, T_Acc& acc) const
        {
            static constexpr unsigned numDims = traits::NumDims< T_Obj >::value;
            auto idx = types::Vec<numDims>::all(0);
            return doGetPtr(obj, acc, idx);
        }

        template< class T_Obj, class T_Acc >
        bool checkPtr(T_Obj& obj, T_Acc& acc, bool isR2CInplaceInput) const
        {
            static constexpr unsigned numDims = traits::NumDims< T_Obj >::value;
            auto idx = types::Vec<numDims>::all(0);
            Ptr startPtr = doGetPtr(obj, acc, idx);
            policies::GetExtents<T_Obj> extents(obj);
            // Basically set check dimensions to the end-index and check if the returned pointer matches the expected one
            unsigned factor = 1;
            for(unsigned i=numDims; i>0; --i)
            {
                unsigned j = i-1;
                startPtr += (extents[j]-1) * factor;
                if(isR2CInplaceInput && i == numDims)
                    factor *= (extents[j] / 2 + 1) * 2;
                else
                    factor *= extents[j];
                idx[j] = extents[j]-1;
                Ptr isPtr = doGetPtr(obj, acc, idx);
                if(startPtr != isPtr)
                    return false;
            }
            return true;
        }
    };

    template< typename T_Ptr >
    struct FFT_Memory_GetPtr< T_Ptr, false >
    {
        using Ptr = T_Ptr;
    private:
        template< class T_Obj, class T_Acc, class T_Idx >
        Ptr
        doGetPtr(T_Obj& obj, T_Acc& acc, T_Idx& idx) const
        {
            auto& ref = acc(idx, obj);
            return policies::safe_ptr_cast<Ptr>(std::make_pair(&ref.real, &ref.imag));
        }
    public:

        /**
         * Returns a pointer to the start of the data in the given object
         * Enabled if we should return a pair of pointers where the first element
         * points to the real part and the second to the imaginary
         *
         * @param obj Object where the data is stored
         * @param acc Accessor to access the elements in @ref obj
         * @return std::pair of pointers to the element at acc({0, ... } obj)
         */
        template< class T_Obj, class T_Acc >
        Ptr
        getPtr(T_Obj& obj, T_Acc& acc) const
        {
            static constexpr unsigned numDims = traits::NumDims< T_Obj >::value;
            auto idx = types::Vec<numDims>::all(0);
            return doGetPtr(obj, acc, idx);
        }

        template< class T_Obj, class T_Acc >
        bool checkPtr(T_Obj& obj, T_Acc& acc) const
        {
            static constexpr unsigned numDims = traits::NumDims< T_Obj >::value;
            auto idx = types::Vec<numDims>::all(0);
            Ptr startPtr = doGetPtr(obj, acc, idx);
            policies::GetExtents<T_Obj> extents(obj);
            // Basically set check dimensions to the end-index and check if the returned pointer matches the expected one
            unsigned factor = 1;
            for(unsigned i=numDims-1; i>0; --i)
            {
                startPtr.first  += extents[i] * factor;
                startPtr.second += extents[i] * factor;
                factor = extents[i];
                idx[i] = extents[i];
                Ptr isPtr = doGetPtr(obj, acc, idx);
                if(startPtr != isPtr)
                    return false;
            }
            return true;
        }
    };

    /**
     * Specialization that does not use own memory but forwards everything to the base object
     */
    template< typename T_Memory>
    struct FFT_Memory< T_Memory, false >:
        public FFT_Memory_GetPtr< std::result_of_t< decltype(&T_Memory::Memory::getData)(typename T_Memory::Memory) > >
    {
        using Memory = T_Memory;

        template< typename T_Extents >
        void init(T_Extents){}

        template< class T_Obj, class T_Acc >
        void copyFrom(T_Obj&, T_Acc&){}

        template< class T_Obj, class T_Acc >
        void copyTo(T_Obj&, T_Acc&){}

        size_t
        getMemSize() const
        {
            return 0;
        }
    };

}  // namespace detail
}  // namespace LiFFT
