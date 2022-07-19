#include "KgWindowing.h"
#include <assert.h>
#include <cstdarg>
#include "KtuMath.h"
#include "windows.h"


KgWindowing::KgWindowing(unsigned frameSize, KeType type, ...)
	: win_(frameSize)
{
	std::va_list args;
	va_start(args, type);
	auto f = functor(type, args);
	va_end(args);

	KtuMath<double>::linspace(0, 1, 0, win_.data(), frameSize);
	for (unsigned i = 0; i < frameSize; i++)
		win_[i] = f(win_[i]);
}


void KgWindowing::porcess(double* x) const
{
	KtuMath<double>::mul(x, win_.data(), x, idim());
}


std::function<double(double)> KgWindowing::functor(KeType type, ...)
{
	switch (type)
	{
	case k_hamming:
		return hamming<double>();

	case k_hann:
		return hann<double>();

	case k_povey:
		return povey<double>();

	case k_blackman:
		return blackman<double>();

	case k_blackmanharris:
		return blackmanharris<double>();

	case k_blackmanharris7:
		return blackmanharris7<double>();

	case k_flattop:
		return flattop<double>();

	case k_triangular:
		return triangular<double>();

	case k_rectangle:
		return [](double) { return 1; };

	case k_rcostaper:
	{
		std::va_list args;
		va_start(args, type);
		auto w = rcostaper<double>(va_arg(args, double));
		va_end(args);
		return w;
	}

	case k_kaiser:
	{
		std::va_list args;
		va_start(args, type);
		auto w = kaiser<double>(va_arg(args, double));
		va_end(args);
		return w;
	}

	}

	return [](double) { return KtuMath<double>::nan; };
}