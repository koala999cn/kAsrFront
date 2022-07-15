#pragma once

// 实数fft，基于fftw3实现

class KgFftw
{
public:

    KgFftw(unsigned sizeT);
    ~KgFftw();

    double* forward(const double* in) const;

    double* backward(const double* in) const;

    unsigned sizeT() const { return sizeT_; }

    unsigned sizeF() const { return sizeT() / 2 + 1; }

private:
    unsigned sizeT_;
    void *dptr_, *in_, *out_;
};
