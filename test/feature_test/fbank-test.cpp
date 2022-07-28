#include "feature/KgFbankPipe.h"
#include "capture/KgWindowing.h"
#include "test-util.h"
#include "base/KtuMath.h"


void fbank_test(const std::vector<double>& wav)
{
	printf(" fbank test...  \n");

	KgFbankPipe::KpOptions opts;
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

	KgFbankPipe fbank(opts);

	matrixd mat;
	unsigned out_size = fbank.odim();
	auto fbank_handler = [&mat, &out_size](double* out) {
		vectord v;
		v.assign(out, out + out_size);
		mat.push_back(v);
	};

	fbank.setHandler(fbank_handler);
	fbank.process(wav.data(), wav.size());
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/fbank-plain.txt");
	dump_bias(mat, kaldi);
	
	opts.useEnergy= KgPreprocess::k_use_energy_win;
	opts.preemphasis = 0.97;
	opts.removeDcOffset = true;
	opts.roundToPower2 = true;
	opts.windowType = KgWindowing::k_povey;
	{
		mat.clear();
		KgFbankPipe fbank(opts);
		out_size = fbank.odim();
		fbank.setHandler(fbank_handler);
		fbank.process(wav.data(), wav.size());
	}
	printf("  test with kaldi preprocessed...  ");
	kaldi = load_matrix("../data/fbank-prep.txt");
	dump_bias(mat, kaldi);

	//printf(":)passed.\n");
}
