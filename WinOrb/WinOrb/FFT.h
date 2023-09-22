#ifndef FFT_H
#define FFT_H

//SUMMONING THE MAFFS GODS
#include <complex>
#include <vector>

typedef std::complex<float> fcomplex;
typedef std::vector<fcomplex> complex_sample;

complex_sample FFT(const complex_sample& sample);
complex_sample IFFT(const complex_sample& sample);

#endif //!FFT_H