#include "feature/KgShiftedDelta.h"
#include "test-util.h"


void delta_sdc_test()
{
	printf(" deltas-sdc test...  ");
	auto mfcc = load_matrix("../data/mfcc-plain.txt");
	KgShiftedDelta sdc(1, 7, 3);
	auto mat = sdc.compute(mfcc);
	mfcc = load_matrix("../data/mfcc-plain-sdc.txt");
	equal_test(mat, mfcc);
	printf("passed.\n");
}
