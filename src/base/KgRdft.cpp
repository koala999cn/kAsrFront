#include "KgRdft.h"

#define FFT_DATA_TYPE double
#include "praat/NUM2.h"
#include "KtuMath.h"
#include <assert.h>


#define PRAAT_TABLE ((autoNUMfft_Table*)internalTable_)


KgRdft::KgRdft(unsigned sizeT, bool kaldiCompat, bool normalize) 
    : sizeT_(sizeT)
    , kaldiCompat_(kaldiCompat)
    , normalize_(normalize)
{
    internalTable_ = new autoNUMfft_Table;
    ::NUMfft_Table_init(PRAAT_TABLE, sizeT);
    assert(PRAAT_TABLE->n == sizeT);
}


KgRdft::~KgRdft()
{
    delete PRAAT_TABLE;
}


void KgRdft::forward(double data[]) const
{
    ::NUMfft_forward(PRAAT_TABLE, data);

    if(kaldiCompat_) { // 调整结果布局，以兼容NR fft
        if (sizeT() > 1) {
            // To be compatible with old behaviour
            double tmp = data[sizeT() - 1];
            for (decltype(sizeT()) i = sizeT() - 1; i > 1; i--)
                data[i] = data[i - 1];

            data[1] = tmp;
        }
    }
}

void KgRdft::backward(double data[]) const
{
    if(kaldiCompat_) {
        if (sizeT() > 1) {
            // To be compatible with old behaviour
            double tmp = data[1];
            for (decltype(sizeT()) i = 1; i < sizeT() - 1; i++) 
                data[i] = data[i + 1];

            data[sizeT() - 1] = tmp;
        }
    }

    ::NUMfft_backward(PRAAT_TABLE, data);

    if (normalize_) // data[i] /= N
        KtuMath<double>::scale(data, sizeT(), static_cast<double>(1.0 / sizeT()));
}

void KgRdft::powerSpectrum(const double *fft/*in*/, double* spec/*out*/) const
{
    // now we have in waveform, first half of complex spectrum
    // it's stored as [real0, realN/2-1, real1, im1, real2, im2, ...]

    auto half_dim = sizeT() / 2;

    spec[0] = fft[0] * fft[0]; // first_energy

    if(!kaldiCompat_) {
        for (decltype(half_dim) i = 1; i < half_dim; i++) {
            double real = fft[i * 2 - 1], im = fft[i * 2];
            spec[i] = real * real + im * im;
        }

        // last_energy, handle this special case
        // Will actually never be used, and anyway
        // if the signal has been bandlimited sensibly this should be zero.
        spec[half_dim] = fft[sizeT() - 1] * fft[sizeT() - 1];

    }
    else {
        for (decltype(half_dim) i = 1; i < half_dim; i++) {
            double real = fft[i * 2], im = fft[i * 2 + 1];
            spec[i] = real * real + im * im;
        }

        spec[half_dim] = fft[1] * fft[1];  // handle this special case
    }

    if (normalize_) {
        KtuMath<double>::forEach(spec, sizeF(), [](double x) { return std::sqrt(x); });
        KtuMath<double>::scale(spec, sizeF(), static_cast<double>(2.0 / sizeT()));
        spec[0] /= 2;
    }
}

