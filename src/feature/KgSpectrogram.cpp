#if 0

#include "KgSpectrogram.h"
#include "KgSpectrum.h"
#include "base/KtuBitwise.h"


KgSpectrogram::KgSpectrogram(const KpOptions& opts)
{
	energyFloor_ = opts.energyFloor;
	auto fftFramingSize = opts.roundToPower2 ? 
		KtuBitwise<unsigned>::ceilPower2(opts.frameSize) : opts.frameSize;

	// KgPreprocess使用原尺寸
	prep_ = std::make_unique<KgPreprocess>(opts); 
	prep_->setHandler([this](double* frame, double energy) {
		std::vector<double> spec(((KgSpectrum*)dptr_)->odim());
		processOneFrame_(frame, spec.data());
		if (prep_->options().energyMode != KgPreprocess::k_energy_none)
			spec[0] = fixEnergy_(energy);
		handler_(spec.data());
		});

	// KgSpectrum使用rounded尺寸
	auto spec = new KgSpectrum(fftFramingSize, opts.sampleRate, KgSpectrum::KeNormMode(opts.norm));
	spec->setType(KgSpectrum::KeType(opts.type));
	dptr_ = spec;
}


KgSpectrogram::~KgSpectrogram()
{
	delete (KgSpectrum*)dptr_;
}


void KgSpectrogram::setHandler(std::function<void(double* spec)> h)
{
	handler_ = h;
}

void KgSpectrogram::process(const double* buf, unsigned len) const
{
	prep_->process(buf, len);
}


void KgSpectrogram::flush() const
{
	prep_->flush();
}


unsigned KgSpectrogram::odim() const
{
	return ((KgSpectrum*)dptr_)->odim();
}



void KgSpectrogram::processOneFrame_(double* in, double* out) const
{
	// assert(in->size() == prep_->odim());
	// assert(out->size() == ((KgSpectrum*)dptr_)->odim();

	// 此处传入的in是原始尺寸，根据需要roundToPower2，以作为fft的输入
	auto frame = in;
	auto spec = (KgSpectrum*)dptr_;
	std::vector<double> buf;
	if (prep_->odim() != spec->idim()) { // roundToPower2
		buf.resize(spec->idim());
		std::copy(frame, frame + prep_->odim(), buf.begin());
		std::fill(buf.begin() + prep_->odim(), buf.end(), 0);
		frame = buf.data();
	}

	spec->porcess(frame);

	std::copy(frame, frame + spec->odim(), out); // TODO: 此处多了一次拷贝
}


double KgSpectrogram::fixEnergy_(double energy) const
{
	auto spec = (KgSpectrum*)dptr_;
	auto norm = spec->normMode();
	double factor(1);
	if (norm == KgSpectrum::k_norm_default)
		factor = 1. / spec->idim();
	else if (norm == KgSpectrum::k_norm_praat)
		factor = 1. / prep_->options().sampleRate;
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

	if (energyFloor_ != 0 && energy < energyFloor_)
		energy = energyFloor_;

	return energy;
}

#endif