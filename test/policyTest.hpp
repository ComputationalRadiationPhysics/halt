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

#include "policyExample.hpp"
#include <chrono>
#include <vector>
#include "liFFT/mem/DataContainer.hpp"
#include "liFFT/mem/RealValues.hpp"
#include "liFFT/mem/ComplexAoSValues.hpp"
#include "liFFT/mem/ComplexSoAValues.hpp"


using millisecs = std::chrono::duration<unsigned long long, std::milli>;
using std::chrono::duration_cast;
using MyClock = std::chrono::high_resolution_clock;

constexpr unsigned NumVals = 1e7;

template< class T >
bool checkResult(const T& result, unsigned expected){
    bool ok = true;
    for(auto val:result){
        if(val != 4*4+3*3){
            ok = false;
            break;
        }
    }

    return ok;
}

void test(){
    using namespace LiFFT::types;
    using namespace LiFFT::mem;
    using LiFFT::calcIntensity;
    using LiFFT::calcIntensity2;

    DataContainer< 1, RealValues<double> > simpleRealData;
    DataContainer< 1, ComplexAoSValues<double> > complexAoS;
    DataContainer< 1, ComplexSoAValues<double> > complexSoA;

    simpleRealData.extents = {NumVals};
    simpleRealData.data = new Real<double>[NumVals];
    for(unsigned i=0; i<NumVals; i++)
        simpleRealData.data[i] = 5;

    complexAoS.extents = {NumVals};
    complexAoS.data = new Complex<double>[NumVals];
    for(unsigned i=0; i<NumVals; i++){
        complexAoS.data[i].real = 4;
        complexAoS.data[i].imag = 3;
    }

    complexSoA.extents = {NumVals};
    complexSoA.data.getRealData() = new Real<double>[NumVals];
    complexSoA.data.getImagData() = new Real<double>[NumVals];
    for(unsigned i=0; i<NumVals; i++){
        complexSoA.data.getRealData()[i] = 4;
        complexSoA.data.getImagData()[i] = 3;
    }

    std::vector<double> result(NumVals);

    MyClock timer;
    auto t1 = timer.now();
    calcIntensity(simpleRealData, result.data());
    if(!checkResult(result, 5*5))
        std::cout << "Check failed :(";
    calcIntensity(complexAoS, result.data());
    if(!checkResult(result, 4*4+3*3))
        std::cout << "Check failed :(";
    calcIntensity(complexSoA, result.data());
    if(!checkResult(result, 4*4+3*3))
        std::cout << "Check failed :(";
    auto t2 = timer.now();
    std::cout << "Time used for 1: " << duration_cast<millisecs>(t2-t1).count() << "ms\n";

    t1 = timer.now();
    calcIntensity2(simpleRealData, result.data());
    if(!checkResult(result, 5*5))
        std::cout << "Check failed :(";
    calcIntensity2(complexAoS, result.data());
    if(!checkResult(result, 4*4+3*3))
        std::cout << "Check failed :(";
    calcIntensity2(complexSoA, result.data());
    if(!checkResult(result, 4*4+3*3))
        std::cout << "Check failed :(";
    t2 = timer.now();
    std::cout << "Time used for 2: " << duration_cast<millisecs>(t2-t1).count() << "ms\n";
}
