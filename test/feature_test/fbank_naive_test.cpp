#include "feature/KgFbank.h"
#include "test-util.h"
#include "base/KtuMath.h"


// ֱ�Ӽ���kaldi��spectrogram��������fbank

void fbank_naive_test()
{
	printf(" fbank naive test...  \n");

	auto spec = load_matrix("../data/spectrogram-kaldi-plain.txt");
	for (auto& r : spec)
		for (auto& c : r)
			c = exp(c); // ��log��ת��Ϊpower��

	KgFbank::KpOptions fbankOpts;
	fbankOpts.type = KgFbank::k_mel; 
	fbankOpts.lowFreq = 20;
	fbankOpts.highFreq = 0;
	fbankOpts.numBins = 23;
	fbankOpts.normalize = false;
	KgFbank fbank(16000, spec[0].size(), fbankOpts);
	matrixd mat;
	for (auto& i : spec) {
		std::vector<double> out(fbank.odim());
		fbank.process(i.data(), out.data());
		KtuMath<double>::applyFloor(out.data(), out.size(), std::numeric_limits<double>::epsilon());
		KtuMath<double>::applyLog(out.data(), out.size());
		mat.push_back(out);
	}
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/fbank-plain.txt");
	dump_bias(mat, kaldi);
}
