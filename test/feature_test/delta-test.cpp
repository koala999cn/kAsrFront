#include "feature/KgDelta.h"
#include "test-util.h"


void delta_test()
{
	printf(" add-deltas test...  ");
	auto mfcc = load_matrix("../data/mfcc-plain.txt");

	KgDelta::KpOptions opts;
	opts.idim = mfcc[0].size();
	opts.order = 2;
	opts.window = 2;
	KgDelta delta(opts);
	auto mat = delta.compute(mfcc);

	mfcc = load_matrix("../data/mfcc-plain-deltas.txt");
	equal_test(mat, mfcc);
	printf("passed.\n");
}
