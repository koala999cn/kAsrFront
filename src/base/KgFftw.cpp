#include "KgFftw.h"
#include "fftw/fftw3.h"
#include "KtuMath.h"
#include <assert.h>


KgFftw::KgFftw(unsigned sizeT)
    : sizeT_(sizeT)
{
    in_ = fftw_malloc(sizeof(double) * sizeT);
    out_ = fftw_malloc(sizeof(fftw_complex) * sizeF());
    dptr_ = fftw_plan_dft_r2c_1d(sizeT, (double*)in_, (fftw_complex*)out_, FFTW_MEASURE);
}


KgFftw::~KgFftw()
{
    fftw_destroy_plan((fftw_plan)dptr_);
    fftw_free(in_);
    fftw_free(out_);
}


double* KgFftw::forward(const double* in) const
{
    std::copy(in, in + sizeT(), (double*)in_); // TODO: 多一次拷贝错做
    fftw_execute(fftw_plan(dptr_));
    return (double*)out_;
}


double* KgFftw::backward(const double* in) const
{
    return 0;
}

