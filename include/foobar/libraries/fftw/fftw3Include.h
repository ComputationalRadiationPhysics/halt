/*
 * Workaround NVCC pretending to be gcc but not supporting float128
 * https://github.com/FFTW/fftw3/issues/18
 */
#ifdef __CUDACC__
    #define __ICC
#endif
#include "fftw3.h"
#ifdef __CUDACC__
    #undef __ICC
#endif
