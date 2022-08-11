#include "feature/KgDelta.h"
#include "test-util.h"


void online_delta_test()
{
	printf(" online add-deltas test...  ");
	auto mfcc = load_matrix("../data/mfcc-plain.txt");

	KgDelta::KpOptions opts;
	opts.idim = mfcc[0].size();
	opts.order = 2;
	opts.window = 2;
	KgDelta delta(opts);
	std::vector<std::vector<double>> mat;
	for (unsigned i = 0; i < mfcc.size(); i++) {
		std::vector<double> out(delta.odim());
		delta.process(mfcc[i].data(), out.data());
		mat.push_back(out);
	}

	mfcc = load_matrix("../data/mfcc-plain-deltas.txt");

	// 在线计算滞后order*window
	mat.erase(mat.begin(), mat.begin() + 4);
	mfcc.erase(mfcc.end() - 4, mfcc.end());

	equal_test(mat, mfcc);
	printf("passed.\n");
}
