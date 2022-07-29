#include "KgFbankPipe.h"
#include "base/KtuBitwise.h"
#include "base/KtuMath.h"


KgFbankPipe::KgFbankPipe(const KpOptions& opts)
	: super_(opts,
		KgSpectrum::KpOptions{
			opts.frameSize,
			opts.sampleRate,
			opts.specType,
			opts.specNorm,
			opts.roundToPower2
		},
		KgFbank::KpOptions{
			opts.sampleRate,
			KgSpectrum::odim(opts.frameSize, opts.roundToPower2),
			opts.bankType,
			opts.numBanks,
			opts.lowFreq,
			opts.highFreq,
			opts.bankNorm
		})
	, energyFloor_(opts.energyFloor) {

	// �����������ɹ���
	prep_->setHandler([this](double* frame, double energy) {
		std::vector<double> out(odim());
		double* p = out.data();

		if (prep_->options().useEnergy) {
			*p = energy;
			pipeline_.get<0>().fixPower(p, 1, false); // ��������ֵ
			if (*p < energyFloor_)
				*p = energyFloor_;
			++p;
		}

		pipeline_.process(frame, p);
		KtuMath<double>::applyFloor(out.data(), odim(), std::numeric_limits<double>::epsilon());
		KtuMath<double>::applyLog(out.data(), odim());

		handler_(out.data());
	});
}


unsigned KgFbankPipe::odim() const 
{
	return prep_->options().useEnergy ? super_::odim() + 1 : super_::odim();
}