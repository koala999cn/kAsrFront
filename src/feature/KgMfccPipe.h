#pragma once
#include "KgFbankPipe.h"
#include "KgMfcc.h"


class KgMfccPipe : public KtFeatPipeline<KgSpectrum, KgFbank, KgMfcc>
{
public:
	using super_ = KtFeatPipeline<KgSpectrum, KgFbank, KgMfcc>;


	struct KpOptions : public KgFbankPipe::KpOptions
	{
		// ��KgPreprocess::useEnergy��k_use_energy_none, ��ʹ���ź������滻c0

		unsigned numCeps;  
		double cepsLifter; 
	};

	KgMfccPipe(const KpOptions& opts);

private:
	double energyFloor_;
};
