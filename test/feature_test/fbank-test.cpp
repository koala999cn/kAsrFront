#include "feature/KgFbank.h"
#include "feature/KgSpectrogram.h"
#include "feature/KgSpectrum.h"
#include "capture/KgWindowing.h"
#include "test-util.h"
#include "base/KtuMath.h"


void fbank_test(const std::vector<double>& wav)
{
	printf(" fbank test...  \n");

	KgSpectrogram::KpOptions opts;
	opts.sampleRate = 16000;
	opts.dither = false;
	opts.frameSize = 0.025 * 16000;
	opts.frameShift = 0.01 * 16000;
	opts.preemphasis = 0;
	opts.removeDcOffset = false;
	opts.windowType = KgWindowing::k_rectangle;
	opts.energyMode = KgPreprocess::k_energy_raw;
	opts.roundToPower2 = false;
	opts.type = KgSpectrum::k_power; // kaldi使用功率谱或mag谱计算fbank
	opts.norm = KgSpectrum::k_norm_kaldi;
	opts.energyFloor = 1;
	KgSpectrogram spec(opts);

	matrixd mat;
	unsigned spec_size = spec.odim();
	auto spec_handler = [&mat, &spec_size](double* spec) -> bool {
		vectord v;
		v.assign(spec, spec + spec_size);
		mat.push_back(v);
		return true;
	};

	spec.process(wav.data(), wav.size(), spec_handler);

	KgFbank::KpOptions fbankOpts;
	fbankOpts.type = KgFbank::k_mel; // kaldi使用mel尺度计算fbank
	fbankOpts.lowFreq = 20;
	fbankOpts.highFreq = 0;
	fbankOpts.numBins = 23;
	fbankOpts.normalize = false;
	KgFbank fbank(16000, spec.odim(), fbankOpts);
	for (unsigned i = 0; i < mat.size(); i++) {
		vectord v(fbank.odim());
		fbank.process(mat[i].data(), v.data());
		KtuMath<double>::applyFloor(v.data(), v.size(), std::numeric_limits<double>::epsilon());
		KtuMath<double>::applyLog(v.data(), v.size());
		mat[i] = v;
	}

	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/fbank-plain.txt");
	dump_bias(mat, kaldi);

	mat.clear();
	opts.energyMode = KgPreprocess::k_energy_post;
	opts.preemphasis = 0.97;
	opts.removeDcOffset = true;
	opts.roundToPower2 = true;
	opts.windowType = KgWindowing::k_povey;
	{
		KgSpectrogram spec(opts);
		spec_size = spec.odim();
		spec.process(wav.data(), wav.size(), spec_handler);
		KgFbank fbank(16000, spec.odim(), fbankOpts);
		for (unsigned i = 0; i < mat.size(); i++) {
			vectord v(fbank.odim());
			fbank.process(mat[i].data(), v.data());
			KtuMath<double>::applyFloor(v.data(), v.size(), std::numeric_limits<double>::epsilon());
			KtuMath<double>::applyLog(v.data(), v.size());
			mat[i] = v;
		}
	}
	printf("  test with kaldi preprocessed...  ");
	kaldi = load_matrix("../data/fbank-prep.txt");
	dump_bias(mat, kaldi);

	printf(":)passed.\n");
}
