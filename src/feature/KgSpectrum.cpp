#include "KgSpectrum.h"
#include "KgRdft.h"
#include "KtuMath.h"


KgSpectrum::KgSpectrum(unsigned frameSize)
	: roundToPower2_(true)
	, type_(k_power)
	, floor_(1e-12f) // TODO: 重新设置
{
	rdft_ = new KgRdft(frameSize); // TODO: 处理roundToPower2_
}


KgSpectrum::~KgSpectrum()
{
	delete (KgRdft*)rdft_;
}


std::pair<unsigned, unsigned> KgSpectrum::dim() const
{
	auto rdft = ((KgRdft*)rdft_);
	return { rdft->sizeT(), rdft->sizeF() };
}


void KgSpectrum::reset(unsigned frameSize)
{
	auto rdft = ((KgRdft*)rdft_);
	if (rdft->sizeT() != frameSize) {
		delete rdft;
		rdft_ = new KgRdft(frameSize);
	}
}


void KgSpectrum::porcess(double* data) const
{
	auto rdft = ((KgRdft*)rdft_);
	rdft->forward(data);
	rdft->powerSpectrum(data); // 功率谱

	// 转换为其他类型谱
	auto c = rdft->sizeT();
	if (type_ == k_mag) {
		for (unsigned n = 0; n < c; n++)
			data[n] = sqrt(data[n]);
	}
	else if (type_ == k_log) {
		KtuMath<double>::applyFloor(data, c, floor_);
		KtuMath<double>::applyLog(data, c);
	}
	else if (type_ == k_db) {
		for (unsigned n = 0; n < c; n++) {
			data[n] = std::max(data[n], floor_);
			data[n] = 10 * log10(data[n]);
		}
	}
}
