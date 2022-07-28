#pragma once
#include "KtFeatPipeline.h"
#include "KgSpectrum.h"


class KgSpectrogram : public KtFeatPipeline<KgSpectrum>
{
public:
	using super_ = KtFeatPipeline<KgSpectrum>;

	struct KpOptions : public KgPreprocess::KpOptions
	{
		// ��KgPreprocess::useEnergy��k_use_energy_none, ��c0��energy�滻

		KgSpectrum::KeType type;
		KgSpectrum::KeNormMode norm;
		bool roundToPower2;
		double energyFloor; // ��energyС�ڸ�ֵʱ���ø�ֵ����ź�energyֵ����norm�߶����
	};

	KgSpectrogram(const KpOptions& opts);

private:
	double energyFloor_;
};
