#include "feature/KgSpectrum.h"
#include "base/KtuMath.h"
#include "test-util.h"


void spectrum_test(const std::vector<double>& wav)
{
	printf(" spectrum test...  \n");

	KgSpectrum::KpOptions opts;
	opts.sampleRate = 16000;
	opts.frameSize = wav.size();
	opts.norm = KgSpectrum::k_norm_praat;
	opts.type = KgSpectrum::k_power; // praat为power谱
	opts.roundToPower2 = false;
	KgSpectrum spec(opts);
	std::vector<double> out(spec.odim());
	spec.process(wav.data(), out.data());
	
	printf("  test with praat...  ");
	auto praat = load_matrix("../data/spectrum-praat.txt");
	if (praat.size() != out.size()) {
		printf("spectrum-praat.txt missing or corupt.");
		test_failed();
	}
	equal_test(out, get_column(praat, 0));
	printf(":)passed.\n");

	double energy = KtuMath<double>::sum2(wav.data(), wav.size());
	{
		opts.norm = KgSpectrum::k_norm_kaldi;
		opts.type = KgSpectrum::k_log; // kaldi为log谱
		KgSpectrum spec(opts);
		spec.process(wav.data(), out.data());
	}
	out[0] = std::log(energy * std::numeric_limits<std::int16_t>::max() * std::numeric_limits<std::int16_t>::max()); // 按kaldi模式修正spec[0]
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/spectrum-kaldi-plain.txt");
	if (kaldi.size() != 1 || kaldi[0].size() != out.size()) {
		printf("spectrum-kaldi-plain.txt missing or corupt.");
		test_failed();
	}
	dump_bias(out, kaldi.front());
}

