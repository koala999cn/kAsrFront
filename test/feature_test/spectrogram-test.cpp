#include "feature/KgSpectrogram.h"
#include "feature/KgSpectrum.h"
#include "capture/KgWindowing.h"
#include "test-util.h"


void make_plain(KgPreprocess::KpOptions& opts);

void spectrogram_test(const std::vector<double>& wav)
{
	printf(" spectrogram test...  \n");

	matrixd mat;
	unsigned spec_size;
	auto spec_handler = [&mat, &spec_size](double* spec) -> bool {
		vectord v;
		v.assign(spec, spec + spec_size);
		mat.push_back(v);
		return true;
	};

	KgSpectrogram::KpOptions opts;
	make_plain(opts);

	opts.windowType = KgWindowing::k_rectangle;
	opts.energyMode = KgPreprocess::k_energy_raw;
	opts.roundToPower2 = false;
	opts.type = KgSpectrum::k_log;
	opts.norm = KgSpectrum::k_norm_kaldi;
	opts.energyFloor = 1;
	KgSpectrogram spec(opts);
	spec_size = spec.odim();
	spec.process(wav.data(), wav.size(), spec_handler);
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/spectrogram-kaldi-plain.txt");
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
		//spec.flush(spec_handler);
	}
	printf("  test with kaldi preprocessed...  ");
	kaldi = load_matrix("../data/spectrogram-kaldi-prep.txt");
	dump_bias(mat, kaldi);

	printf(":)passed.\n");
}
