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
		// ��KgPreprocess::useEnergy��k_use_energy_none, ��Fbank�����һ����ά��

		KgFbank::KeType bankType;
		unsigned numBanks;
		double lowFreq;
		double highFreq;
		bool bankNorm; // ��true����ÿ��bank���˲����������bank���(Hertz��)

		KgSpectrum::KeType specType;
		KgSpectrum::KeNormMode specNorm;
		bool roundToPower2;
		double energyFloor; // ��energyС�ڸ�ֵʱ���ø�ֵ����ź�energyֵ����norm�߶����
	};

	KgFbankPipe(const KpOptions& opts);

	unsigned odim() const;

private:
	double energyFloor_;
};
