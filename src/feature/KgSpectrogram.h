#pragma once
#include "KtFeatPipeline.h"
#include "KgSpectrum.h"


class KgSpectrogram : public KtFeatPipeline<KgSpectrum>
{
public:
	using super_ = KtFeatPipeline<KgSpectrum>;


	struct KpOptions : public KgPreprocess::KpOptions
	{
		KgSpectrum::KeType type;
		KgSpectrum::KeNormMode norm;
		bool roundToPower2;
		double energyFloor; // 当energy小于该值时，用该值替代信号energy值，与norm高度相关
	};

	KgSpectrogram(const KpOptions& opts)
		: super_(opts, KgSpectrum::KpOptions{ 
		     opts.frameSize, 
			 opts.sampleRate, 
			 opts.type, 
			 opts.norm, 
			 opts.roundToPower2 
			})
	    , energyFloor_(opts.energyFloor) {

		// 重置特征生成过程
		prep_->setHandler([this](double* frame, double energy) {
			std::vector<double> out(odim());
			pipeline_.process(frame, out.data());
			if (prep_->options().useEnergy != KgPreprocess::k_use_energy_none) {
				out[0] = energy;
				pipeline_.get<0>().fixPower(out.data(), 1, false); // 修正能量值
				if (out[0] < energyFloor_)
					out[0] = energyFloor_;
			}

			handler_(out.data());
			});
	}

private:
	double energyFloor_;
};