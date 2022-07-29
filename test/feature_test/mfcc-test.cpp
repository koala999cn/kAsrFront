#include "feature/KgMfccPipe.h"
#include "capture/KgWindowing.h"
#include "test-util.h"
#include "base/KtuMath.h"


void mfcc_test(const std::vector<double>& wav)
{
	printf(" mfcc test...  \n");

	KgMfccPipe::KpOptions opts;
	opts.sampleRate = 16000;
	opts.dither = false;
	opts.frameSize = 0.025 * 16000;
	opts.frameShift = 0.01 * 16000;
	opts.preemphasis = 0;
	opts.removeDcOffset = false;
	opts.windowType = KgWindowing::k_rectangle;
	opts.useEnergy = KgPreprocess::k_use_energy_none;
	opts.roundToPower2 = false;
	opts.specType = KgSpectrum::k_power; // kaldi使用功率谱或mag谱计算fbank
	opts.specNorm = KgSpectrum::k_norm_kaldi;
	opts.energyFloor = 1;
	opts.bankType = KgFbank::k_mel; // kaldi使用mel尺度计算fbank
	opts.lowFreq = 20;
	opts.highFreq = 0;
	opts.numBanks = 23;
	opts.bankNorm = false;
	opts.numCeps = 13;
	opts.cepsLifter = 0;

	KgMfccPipe mfcc(opts);
	matrixd mat;
	unsigned mfcc_size = mfcc.odim();
	auto mfcc_handler = [&mat, &mfcc_size](double* mfcc) {
		vectord v;
		v.assign(mfcc, mfcc + mfcc_size);
		mat.push_back(v);
		return true;
	};
	mfcc.setHandler(mfcc_handler);
	mfcc.process(wav.data(), wav.size());
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/mfcc-plain.txt");
	dump_bias(mat, kaldi);


	mat.clear();
	opts.preemphasis = 0.97;
	opts.removeDcOffset = true;
	opts.roundToPower2 = true;
	opts.cepsLifter = 22;
	opts.windowType = KgWindowing::k_povey;
	{
		KgMfccPipe mfcc(opts);
		mfcc_size = mfcc.odim();
		mfcc.setHandler(mfcc_handler);
		mfcc.process(wav.data(), wav.size());
	}
	printf("  test with kaldi preprocessed...  ");
	kaldi = load_matrix("../data/mfcc-prep.txt");
	dump_bias(mat, kaldi);

	//printf(":)passed.\n");
}
