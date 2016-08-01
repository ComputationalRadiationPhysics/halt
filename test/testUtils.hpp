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

#include "testDefines.hpp"
#include "liFFT/traits/IdentityAccessor.hpp"
#include "liFFT/traits/IsComplex.hpp"
#include "liFFT/policies/Loop.hpp"
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <iostream>

#define TEST(function) if(!(function)) return 1

namespace LiFFTTest {

    /**
     * Initializes the test environment and prepares the base line FFTs to compare against
     */
    void init();
    /**
     * Frees all resources used in the test environment
     */
    void finalize();
    /**
     * Executes the base tests and outputs PDFs visualizing the results
     */
    void visualizeBase();

    enum class BaseInstance{
        InR2C, OutR2C,
        InC2C, OutC2C
    };

    /**
     * Outputs one of the base data containers to a file (pdf+txt)
     *
     * @param inst Which one to output
     * @param filePath Output path
     */
    void visualizeOutput(BaseInstance inst, const std::string& filePath);

    /**
     * Executes the FFT on the base Input for R2C
     */
    void execBaseR2C();
    /**
     * Executes the FFT on the base Input for C2C
     */
    void execBaseC2C();

    /**
     * Maximum error detected during a compare run
     */
    struct CmpError{
        double maxAbsDiff = 0;
        double maxRelDiff = 0;

        CmpError(): maxAbsDiff(0), maxRelDiff(0)
        {}
        CmpError(double allowedAbsDiff, double allowedRelDiff): maxAbsDiff(allowedAbsDiff), maxRelDiff(allowedRelDiff)
        {}

        friend inline std::ostream& operator<<(std::ostream& stream, const CmpError& e){
            stream << "Max AbsDiff = " << e.maxAbsDiff  << " Max RelDiff = " << e.maxRelDiff;
            return stream;
        }
    };


    /**
     * "Functor" used for comparing multidimensional containers of Real or Complex data
     */
    struct CompareFunc
    {
        bool ok = true;
        CmpError e;
        CmpError m_allowed;

        CompareFunc(CmpError allowed): m_allowed(allowed){}

        template< unsigned T_curDim, unsigned T_endDim, class... T_Args>
        void handleLoopPre(T_Args&&...){}
        template< unsigned T_curDim, unsigned T_endDim, class... T_Args>
        void handleLoopPost(T_Args&&...){}

        template<class T, class U>
        std::enable_if_t< LiFFT::traits::IsComplex<T>::value, bool >
        compare(const T& expected, const U& is)
        {
            using Precision = LiFFT::traits::IntegralType_t<T>;
            using Complex = LiFFT::types::Complex<Precision>;
            static_assert(LiFFT::traits::IsBinaryCompatible<T, Complex>::value, "Cannot convert expected");
            static_assert(LiFFT::traits::IsBinaryCompatible<U, Complex>::value, "Cannot convert is");
            const Complex& m_expected = reinterpret_cast<const Complex&>(expected);
            const Complex& m_is = reinterpret_cast<const Complex&>(is);
            return compare(m_expected.real, m_is.real) && compare(m_expected.imag, m_is.imag);
        }

        template<class T, class U>
        std::enable_if_t< !LiFFT::traits::IsComplex<T>::value, bool >
        compare(const T& expected, const U& is)
        {
            if(expected == is)
                return true;
            auto absDiff = std::abs(expected-is);
            if(absDiff <= m_allowed.maxAbsDiff)
                return true;
            auto relDiff = std::abs(absDiff / expected);
            if(relDiff <= m_allowed.maxRelDiff)
                return true;
            if(absDiff > e.maxAbsDiff)
                e.maxAbsDiff = absDiff;
            if(relDiff > e.maxRelDiff)
                e.maxRelDiff = relDiff;
            return false;
        }

        template<
            unsigned T_curDim,
            class T_Index,
            class T_Src,
            class T_SrcAccessor,
            class T_Dst,
            class T_DstAccessor
            >
        void
        handleInnerLoop(const T_Index& idx, const T_Src& expected, T_SrcAccessor&& accExp, T_Dst& is, T_DstAccessor&& accIs)
        {
            if(!compare(accExp(idx, expected), accIs(idx, is)))
                ok = false;
        }
    };

    /**
     * Compares 2 multidimensional containers
     *
     * @param expected   Container with expected values
     * @param is         Container with actual values
     * @param allowedErr Maximum allowed error
     * @param expAcc     Accessor for expected container [IdentityAccessor used]
     * @param isAcc      Accessor for actual container [IdentityAccessor used]
     * @return Pair: 1: bool OK, 2: Maximum errors detected
     */
    template< class T, class U, class T_AccessorT = LiFFT::traits::IdentityAccessor_t<T>, class T_AccessorU = LiFFT::traits::IdentityAccessor_t<U> >
    std::pair< bool, CmpError >
    compare(const T& expected, const U& is, CmpError allowedErr = CmpError(1e-4, 5e-5), const T_AccessorT& expAcc = T_AccessorT(), const T_AccessorU& isAcc = T_AccessorU())
    {
        CompareFunc result(allowedErr);
        LiFFT::policies::loop(expected, result, expAcc, is, isAcc);
        return std::make_pair(result.ok, result.e);
    }

    /**
     * Checks if the results match and calls BOOST_ERROR on failure
     *
     * @param baseRes   Result from base execution (assumed valid)
     * @param res       Data to compare against
     * @param testDescr String the identifies the test
     * @param maxErr    Maximum allowed error
     */
    template< class T_BaseResult, class T_Result >
    void checkResult(const T_BaseResult& baseRes, const T_Result& res, const std::string& testDescr, CmpError maxErr = CmpError(1e-4, 5e-5))
    {
        auto cmpRes = compare(baseRes, res, maxErr);
        if(!cmpRes.first)
            BOOST_ERROR("Error for " << testDescr << ": " << cmpRes.second);
    }

}  // namespace LiFFTTest
