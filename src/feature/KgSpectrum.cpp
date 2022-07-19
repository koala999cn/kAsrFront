#include "KgSpectrum.h"
#include "KgRdft.h"
#include "KtuMath.h"


KgSpectrum::KgSpectrum(unsigned frameSize, double sampleRate, KeNormMode norm)
	: type_(k_power)
	, norm_(norm)
	, f_(sampleRate)
{
	rdft_ = new KgRdft(frameSize, false, k_norm_default == norm);
}


KgSpectrum::~KgSpectrum()
{
	delete (KgRdft*)rdft_;
}


unsigned KgSpectrum::idim() const
{
	return ((KgRdft*)rdft_)->idim();
}


unsigned KgSpectrum::odim() const
{
	return ((KgRdft*)rdft_)->odim();
}


void KgSpectrum::reset(unsigned frameSize)
{
	auto rdft = ((KgRdft*)rdft_);
	if (rdft->idim() != frameSize) {
		delete rdft;
		rdft_ = new KgRdft(frameSize);
	}
}


void KgSpectrum::porcess(double* data) const
{
	auto rdft = ((KgRdft*)rdft_);
	rdft->forward(data);
	rdft->powerSpectrum(data); // 功率谱

	using kMath = KtuMath<double>;
	auto c = rdft->odim();

	// 归一化
	if (norm_ == k_norm_praat)
		kMath::scale(data, c, 1 / (f_ * f_));
	else if (norm_ == k_norm_kaldi) {
		auto int16_max = std::numeric_limits<std::int16_t>::max();
		kMath::scale(data, c, int16_max * int16_max);
	}
	else if (norm_ == k_norm_default) {
		; // KgRdft实现
	}

	// 视情转换为其他类型谱
	if (type_ == k_mag) {
		kMath::forEach(data, c, [](double x) { return std::sqrt(x); });
	}
	else if (type_ == k_log) {
		kMath::applyFloor(data, c, std::numeric_limits<double>::epsilon());
		kMath::applyLog(data, c);
	}
	else if (type_ == k_db) {
		kMath::applyFloor(data, c, std::numeric_limits<double>::epsilon());
		kMath::forEach(data, c, [](double x) { return 10 * std::log10(x); });
	}
}
