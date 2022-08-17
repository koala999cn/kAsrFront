#include "KgMfccPipe.h"
#include "KtuMath.h"


KgMfccPipe::KgMfccPipe(const KpOptions& opts)
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
		},
		KgMfcc::KpOptions{
			opts.numBanks,
			opts.numCeps,
			opts.cepsLifter
		})
	, energyFloor_(opts.energyFloor) {

	// �����������ɹ���
	prep_->setHandler([this](double* frame, double energy) {
		std::vector<double> fbank(pipeline_.get<1>().odim());

		pipeline_.doPipeline<2>(frame, fbank.data()); // ֻ����ǰ������ˮ��Ԫ��
		KtuMath<double>::applyFloor(fbank.data(), static_cast<unsigned>(fbank.size()),
			std::numeric_limits<double>::epsilon());
		KtuMath<double>::applyLog(fbank.data(), static_cast<unsigned>(fbank.size()));

		std::vector<double> out(odim());
		pipeline_.get<2>().process(fbank.data(), out.data());

		if (prep_->options().useEnergy) {
			out[0] = energy;
			pipeline_.get<0>().fixPower(out.data(), 1, false); // ��������ֵ
			KtuMath<double>::applyFloor(out.data(), 1, std::numeric_limits<double>::epsilon());
			KtuMath<double>::applyLog(out.data(), 1);
			if (out[0] < energyFloor_)
				out[0] = energyFloor_;
		}

		handler_(out.data());
	});
}
