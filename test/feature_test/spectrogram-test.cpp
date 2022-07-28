#include "feature/KgSpectrogram.h"
#include "capture/KgWindowing.h"
#include "test-util.h"


void spectrogram_test(const std::vector<double>& wav)
{
	printf(" spectrogram test...  \n");

	matrixd mat;
	unsigned spec_size;
	auto spec_handler = [&mat, &spec_size](double* spec) {
		vectord v;
		v.assign(spec, spec + spec_size);
		mat.push_back(v);
	};

	KgSpectrogram::KpOptions opts;
	opts.sampleRate = 16000;
	opts.dither = false;
	opts.frameSize = 0.025 * 16000;
	opts.frameShift = 0.01 * 16000;
	opts.preemphasis = 0;
	opts.removeDcOffset = false;
	opts.windowType = KgWindowing::k_rectangle;
	opts.useEnergy = KgPreprocess::k_use_energy_raw;
	opts.roundToPower2 = false;
	opts.type = KgSpectrum::k_log; // 对于spectrogram，kaldi始终计算log谱
	opts.norm = KgSpectrum::k_norm_kaldi;
	opts.energyFloor = 1;

	KgSpectrogram spec(opts);
	spec_size = spec.odim();
	spec.setHandler(spec_handler);
	spec.process(wav.data(), wav.size());
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/spectrogram-kaldi-plain.txt");
	dump_bias(mat, kaldi);


	mat.clear();
	opts.useEnergy = KgPreprocess::k_use_energy_win;
	opts.preemphasis = 0.97;
	opts.removeDcOffset = true;
	opts.roundToPower2 = true;
	opts.windowType = KgWindowing::k_povey;
	{
		KgSpectrogram spec(opts);
		spec_size = spec.odim();
		spec.setHandler(spec_handler);
		spec.process(wav.data(), wav.size());
	}
	printf("  test with kaldi preprocessed...  ");
	kaldi = load_matrix("../data/spectrogram-kaldi-prep.txt");
	dump_bias(mat, kaldi);

	//printf(":)passed.\n");
}
