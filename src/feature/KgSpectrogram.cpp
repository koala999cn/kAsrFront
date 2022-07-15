#include "KgSpectrogram.h"
#include "KgSpectrum.h"
#include "base/KtuBitwise.h"


KgSpectrogram::KgSpectrogram(const KpOptions& opts, spcetrum_handler h)
{
	energyFloor_ = opts.energyFloor;
	fftFramingSize_ = opts.roundToPower2 ? 
		KtuBitwise<unsigned>::ceilPower2(opts.frameSize) : opts.frameSize;

	dptr_ = new KgSpectrum(fftFramingSize_, opts.sampleRate, KgSpectrum::KeNormMode(opts.norm));

	auto handler = [h, this](double* frame, double energy) -> bool{
		auto p = frame;
		std::vector<double> buf;
		if (fftFramingSize_ != prep_->dim()) { // roundToPower2
			buf.resize(fftFramingSize_);
			std::copy(frame, frame + prep_->dim(), buf.begin());
			std::fill(buf.begin() + prep_->dim(), buf.end(), 0);
			p = buf.data();
		}

		((KgSpectrum*)dptr_)->porcess(p);

		if (prep_->options().energyMode > 0) 
			buf[0] = fixEnergy_(energy);

		return h(buf.data());
	};

	prep_ = std::make_unique<KgPreprocess>(opts, handler);
}


KgSpectrogram::~KgSpectrogram()
{
	delete (KgSpectrum*)dptr_;
}


void KgSpectrogram::process(const double* buf, unsigned len) const
{
	prep_->process(buf, len);
}


void KgSpectrogram::flush() const
{
	prep_->flush();
}


unsigned KgSpectrogram::dim() const
{
	return ((KgSpectrum*)dptr_)->dim().second;
}


double KgSpectrogram::fixEnergy_(double energy) const
{
	auto spec = (KgSpectrum*)dptr_;
	auto norm = spec->normMode();
	double factor(1);
	if (norm == KgSpectrum::k_norm_default)
		factor = 1. / spec->dim().first;
	else if (norm == KgSpectrum::k_norm_praat)
		factor = 1. / spec->dim().second;
	else if (norm == KgSpectrum::k_norm_kaldi)
		factor = std::numeric_limits<std::int16_t>::max();

	auto type = spec->type();
	if (type == KgSpectrum::k_mag)
		energy *= factor;
	else {
		energy *= factor * factor;
		if (type == KgSpectrum::k_log) {
			if (energy < std::numeric_limits<double>::epsilon())
				energy = std::numeric_limits<double>::epsilon();
			energy = log(energy);
		}
		else if (type == KgSpectrum::k_db) {
			if (energy < std::numeric_limits<double>::epsilon())
				energy = std::numeric_limits<double>::epsilon();
			energy = 10 * log10(energy);
		}
	}

	if (energyFloor_ != 0 && energy > energyFloor_)
		energy = energyFloor_;

	return energy;
}