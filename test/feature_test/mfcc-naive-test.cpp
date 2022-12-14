#include "feature/KgMfcc.h"
#include "test-util.h"


// 直接加载kaldi的fbank输出结果作mfcc

void mfcc_naive_test()
{
	printf(" mfcc naive test...  \n");

	auto fbank = load_matrix("../data/fbank-plain.txt");
	KgMfcc::KpOptions mfccOpts;
	mfccOpts.idim = fbank[0].size();
	mfccOpts.numCeps = 13;
	mfccOpts.cepsLifter = 0;
	KgMfcc mfcc_plain(mfccOpts);
	matrixd mat;
	for (auto& i : fbank) {
		std::vector<double> out(mfcc_plain.odim());
		mfcc_plain.process(i.data(), out.data());
		mat.push_back(out);
	}
	printf("  test with kaldi plain...  ");
	auto kaldi = load_matrix("../data/mfcc-plain.txt");
	dump_bias(mat, kaldi);

	fbank = load_matrix("../data/fbank-prep.txt");
	mfccOpts.idim = fbank[0].size() - 1; // skip c0 (extra energy)
	mfccOpts.cepsLifter = 22;
	KgMfcc mfcc_prep(mfccOpts);
	mat.clear();
	for (auto& i : fbank) {
		std::vector<double> out(mfcc_prep.odim());
		mfcc_prep.process(i.data() + 1, out.data());
		mat.push_back(out);
	}
	printf("  test with kaldi prep...  ");
	kaldi = load_matrix("../data/mfcc-prep.txt");
	dump_bias(mat, kaldi);
}
