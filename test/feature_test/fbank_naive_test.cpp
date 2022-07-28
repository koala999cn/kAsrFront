#include "feature/KgFbank.h"
#include "test-util.h"
#include "base/KtuMath.h"


// 直接加载kaldi的spectrogram输出结果作fbank

void fbank_naive_test()
{
	printf(" fbank naive test...  \n");

	auto spec = load_matrix("../data/spectrogram-kaldi-plain.txt");
	for (auto& r : spec)
		for (auto& c : r)
			c = exp(c); // 从log谱转换为power谱

	KgFbank::KpOptions fbankOpts;
	fbankOpts.sampleRate = 16000;
	fbankOpts.fftBins = unsigned(spec[0].size());
	fbankOpts.type = KgFbank::k_mel; 
	fbankOpts.lowFreq = 20;
	fbankOpts.highFreq = 0;
	fbankOpts.numBanks = 23;
	fbankOpts.normalize = false;
	KgFbank fbank(fbankOpts);
	matrixd mat;
	for (auto& i : spec) {
		std::vector<double> out(fbank.odim());
		fbank.process(i.data(), out.data());
		KtuMath<double>::applyFloor(out.data(), unsigned(out.size()), std::numeric_limits<double>::epsilon());
		KtuMath<double>::applyLog(out.data(), unsigned(out.size()));
		mat.push_back(out);
	}
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/fbank-plain.txt");
	dump_bias(mat, kaldi);
}
