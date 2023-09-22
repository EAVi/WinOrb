#include "FFT.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <corecrt.h>
#include <assert.h>

namespace 
{
	constexpr float tau = 2 * M_PI;
	constexpr fcomplex k_i(0.0f, 1.0f);//imaginary number sqrt(-1);
}

bool is_power_of_two(const size_t& n)
{
	return(n & (n - 1)) == 0;
}

void FFT_Helper(complex_sample& sample)
{
	if (sample.size() <= 1)
	{
		return;
	}
	complex_sample even;
	complex_sample odd;
	for (unsigned i = 0; i < sample.size(); i += 2)//split into odd/even
	{
		even.push_back(sample[i]);
		odd.push_back(sample[i + 1]);
	}

	FFT_Helper(even);//recursion step
	FFT_Helper(odd);


	const unsigned n = sample.size();
	const unsigned n_2 = n / 2;
	fcomplex w = std::exp(tau * k_i/(float)n); //e^(2*pi*i/n)
	//combine back together
	for (unsigned j = 0; j < n_2; ++j)
	{
		sample[j] = even[j] + ((fcomplex)std::pow(w, j) * odd[j]);
		sample[j + n_2] = even[j] - ((fcomplex)std::pow(w, j) * odd[j]);
	}
}
complex_sample FFT(const complex_sample& sample)
{
	size_t n = sample.size();
	assert(is_power_of_two(n));

	complex_sample transformed = sample;
	FFT_Helper(transformed);
	return transformed;
}

void IFFT_Helper(complex_sample& sample)
{
	if (sample.size() <= 1)
	{
		return;
	}
	complex_sample even;
	complex_sample odd;
	for (unsigned i = 0; i < sample.size(); i += 2)//split into odd/even
	{
		even.push_back(sample[i]);
		odd.push_back(sample[i + 1]);
	}

	FFT_Helper(even);//recursion step
	FFT_Helper(odd);

	const unsigned n = sample.size();
	const unsigned n_2 = n / 2;
	fcomplex w = std::exp(-tau * k_i / (float)n); //e^(2*pi*i/n)
	//combine back together
	for (unsigned i = 0; i < n_2; ++i)
	{
		sample[i] = even[i] + ((fcomplex)std::pow(w, i) * odd[i]);
		sample[i + n_2] = even[i] - ((fcomplex)std::pow(w, i) * odd[i]);
	}
}

complex_sample IFFT(const complex_sample& sample)
{
	size_t n = sample.size();
	assert(is_power_of_two(n));

	complex_sample inverted = sample;
	IFFT_Helper(inverted);

	for (fcomplex& val : inverted)
	{
		val /= n;
	}
	return inverted;
}
