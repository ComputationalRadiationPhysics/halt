#pragma once

#include "foobar/mem/RealValues.hpp"
#include "foobar/types/Complex.hpp"
#include "foobar/accessors/ArrayAccessor.hpp"

namespace foobar {
    namespace mem {

        template< typename T, bool T_ownsPointer = true >
        class ComplexSoAValues
        {
        public:
            using type = T;
            static constexpr bool isComplex = true;
            static constexpr bool isAoS = false;
            static constexpr bool ownsPointer = T_ownsPointer;
            using Data = RealValues<T, ownsPointer>;
            using Ptr = typename Data::Ptr;
            using Value = types::Complex<T>;
            using Ref = types::ComplexRef<T>;
            using ConstRef = const types::ComplexRef<T, true>;
            using IdentityAccessor = accessors::ArrayAccessor<>;

            ComplexSoAValues(){}
            ComplexSoAValues(Ptr realData, Ptr imagData, size_t numElements): real_(realData, numElements), imag_(imagData, numElements){}

            void
            reset(std::pair<Ptr, Ptr> data, size_t numElements)
            {
                real_.reset(data.first, numElements);
                imag_.reset(data.second, numElements);
            }

            void
            allocData(size_t numElements)
            {
                real_.allocData(numElements);
                imag_.allocData(numElements);
            }

            void
            freeData()
            {
                real_.freeData();
                imag_.freeData();
            }

            std::pair<Ptr, Ptr>
            releaseData()
            {
                return std::make_pair(real_.releaseData(), imag_.releaseData());
            }

            std::pair<Ptr, Ptr>
            getData() const
            {
                return std::make_pair(real_.getData(), imag_.getData());
            }

            ConstRef
            operator[](size_t idx) const
            {
                return ConstRef(real_[idx], imag_[idx]);
            }

            Ref
            operator[](size_t idx)
            {
                return Ref(real_[idx], imag_[idx]);
            }

            Data&
            getRealData()
            {
                return real_;
            }

            Data&
            getImagData()
            {
                return imag_;
            }

        private:
            Data real_, imag_;
        };

    }  // namespace mem

}  // namespace foobar
