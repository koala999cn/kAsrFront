#pragma once
#include "KtFeatPipeline.h"
#include "KgSpectrum.h"
#include "KgFbank.h"


class KgFbankPipe: public KtFeatPipeline<KgSpectrum, KgFbank>
{
public:
	using super_ = KtFeatPipeline<KgSpectrum, KgFbank>;


	struct KpOptions : public KgPreprocess::KpOptions
	{
		// 若KgPreprocess::useEnergy非k_use_energy_none, 则Fbank输出多一能量维度

		KgFbank::KeType bankType;
		unsigned numBanks;
		double lowFreq;
		double highFreq;
		bool bankNorm; // 若true，则每个bank的滤波结果将除以bank宽度(Hertz域)

		KgSpectrum::KeType specType;
		KgSpectrum::KeNormMode specNorm;
		bool roundToPower2;
		double energyFloor; // 当energy小于该值时，用该值替代信号energy值，与norm高度相关
	};

	KgFbankPipe(const KpOptions& opts);

	unsigned odim() const;

private:
	double energyFloor_;
};
