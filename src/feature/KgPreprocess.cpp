#include "KgPreprocess.h"
#include "KtFraming.h"
#include "KtuMath.h"


namespace kPrivate
{
    struct KgPreprocessInternal_
    {
        KtFraming<double>* framing;
        std::vector<double> window;
    };
}


KgPreprocess::KgPreprocess(const KpOption& option, frame_handler handler)
    : opts_(option)
    , handler_(handler)
{
    auto d = new kPrivate::KgPreprocessInternal_;
    d->framing = new KtFraming<double>(opts_.frameLength, 1, opts_.frameShift);
    dptr_ = d;
}


KgPreprocess::~KgPreprocess()
{
    delete (kPrivate::KgPreprocessInternal_*)dptr_;
}


void KgPreprocess::process(const double* buf, unsigned len) const
{
    auto d = (kPrivate::KgPreprocessInternal_*)dptr_;
    d->framing->apply(buf, buf + len, [this](const double* frame) {
        processOneFrame_(frame);
        });
}


void KgPreprocess::flush() const
{
    auto d = (kPrivate::KgPreprocessInternal_*)dptr_;
    d->framing->flush([this](const double* frame) {
        processOneFrame_(frame);
        });
}


void KgPreprocess::processOneFrame_(const double* frame) const
{
    std::vector<double> newData(opts_.frameLength);
    std::copy(frame, frame + newData.size(), newData.begin());
    double* frame_data = newData.data();
    auto frame_length = newData.size();

    if (opts_.dither != 0)
        ; // TODO: DO dithering

    if (opts_.removeDcOffset)
        KtuMath<double>::subMean(frame_data, frame_length);

    double energy(0);
    if (opts_.useEnergy && opts_.rawEnergy)
        energy = std::max(opts_.energyFloor, KtuMath<double>::dot(frame_data, frame_data, frame_length));

    if (opts_.preemphasis != 0.0)
        ; // TODO: KuFilter::PreEmphasize(frame_data, frame_length, opts.dbPreemphCoeff);

    auto d = (kPrivate::KgPreprocessInternal_*)dptr_;
    if (!d->window.empty()) {
        assert(frame_length == d->window.size());
        KtuMath<double>::mul(frame_data, d->window.data(), frame_data, frame_length);
    }

    if (opts_.useEnergy && !opts_.rawEnergy)
        energy = std::max(opts_.energyFloor, KtuMath<double>::dot(frame_data, frame_data, frame_length));

    handler_(frame_data, energy);
}