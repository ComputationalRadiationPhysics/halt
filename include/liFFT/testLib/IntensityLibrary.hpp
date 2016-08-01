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

namespace LibFoo
{
    void calculateR1D(double* in, double* out, unsigned numEl){
        std::cout << "Real 1D output of " << numEl << " elements" << std::endl;
    }

    void calculateRND(double* in, double* out, unsigned numDims, const unsigned* numEl){
        std::cout << "Real " << numDims << "D output of (";
        for(unsigned i=0; i<numDims; ++i)
            std::cout << numEl[i] << ", ";
        std::cout << ") elements" << std::endl;
    }

    void calculateC1D(double* in, double* out, unsigned numEl){
        std::cout << "Complex 1D output of " << numEl << " elements" << std::endl;
    }

    void calculateCND(double* in, double* out, unsigned numDims, const unsigned* numEl){
        std::cout << "Complex " << numDims << "D output of (";
        for(unsigned i=0; i<numDims; ++i)
            std::cout << numEl[i] << ", ";
        std::cout << ") elements" << std::endl;
    }

    void calculateC1D(double* inReal, double* inImag, double* out, unsigned numEl){
        std::cout << "Complex(SoA) 1D output of " << numEl << " elements" << std::endl;
    }

    void calculateCND(double* inReal, double* inImag, double* out, unsigned numDims, const unsigned* numEl){
        std::cout << "Complex(SoA) " << numDims << "D output of (";
        for(unsigned i=0; i<numDims; ++i)
            std::cout << numEl[i] << ", ";
        std::cout << ") elements" << std::endl;
    }
}  // namespace LibFoo
