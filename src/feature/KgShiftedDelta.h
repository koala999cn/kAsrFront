#pragma once
#include <vector>

// kaldiµÄadd-deltas-sdc

class KgShiftedDelta
{
public:

	typedef std::vector<std::vector<double>> matrixd;

	// delta_window - Size of delta advance and delay.
	// num-blocks - Number of delta blocks in advance of each frame to be concatenated.
	// block-shift - Distance between each block.
	KgShiftedDelta(int delta_window, int num_blocks, int block_shift);

	// The function takes as input a matrix of features and a frame index
	// that it should compute the deltas on.  It puts its output in an object
	// of type VectorBase, of size original-feature-dimension + (1  * num_blocks).
	std::vector<double> process(const matrixd &input_feats, unsigned frame) const;

	matrixd compute(const matrixd &input_feats) const;

private:
	std::vector<double> scales_;  // a scaling window for each
	int num_blocks_, block_shift_;
};

