#include "feature/KgSpectrogram.h"
#include "test-util.h"


void spectrogram_test(const std::vector<double>& wav)
{
	printf(" spectrogram test...  \n");

	matrixd mat;
	auto spec_handler = [&mat](double* spec) -> bool {
		vectord v;
		v.assign(spec, spec + 32); // TODO:
		mat.push_back(v);
	};


	KgSpectrogram::KpOptions opts;

	KgSpectrogram spec(opts, spec_handler);
	spec.process(wav.data(), wav.size());
	spec.flush();

	printf("  test with praat...  ");
	auto praat = load_matrix("../data/spectrum-praat.txt");
	if (praat.size() != data.size()) {
		printf("spectrum-praat.txt missing or corupt.");
		test_failed();
	}
	equal_test(data, get_column(praat, 0));
	printf(":)passed.\n");

	data = wav;
	double energy = KtuMath<double>::sum2(data.data(), data.size());
	{
		KgSpectrum spec1(wav.size(), 16000, KgSpectrum::k_norm_kaldi);
		spec1.setType(KgSpectrum::k_log); // kaldi为log谱
		spec1.porcess(data.data());
		data.resize(spec1.dim().second);
	}
	data[0] = std::log(energy * std::numeric_limits<std::int16_t>::max() * std::numeric_limits<std::int16_t>::max()); // 按kaldi模式修正spec[0]
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/spectrum-kaldi-plain.txt");
	if (kaldi.size() != 1 || kaldi[0].size() != data.size()) {
		printf("spectrum-kaldi-plain.txt missing or corupt.");
		test_failed();
	}
	dump_bias(data, kaldi.front());
	printf(":)passed.\n");
}