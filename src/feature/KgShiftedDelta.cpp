#include "KgShiftedDelta.h"
#include "KtuMath.h"


KgShiftedDelta::KgShiftedDelta(int window, int num_blocks, int block_shift) :
	block_shift_(block_shift),
	num_blocks_(num_blocks)
{
	// Default window is 1.
	scales_.resize(1 + 2 * window);  // also zeros it.
	double normalizer = 0.0;
	for (int j = -window; j <= window; j++) {
		normalizer += j*j;
		scales_[j + window] += static_cast<double>(j);
	}
	KtuMath<double>::scale(scales_.data(), scales_.size(), 1.0 / normalizer);
}


std::vector<double> KgShiftedDelta::process(const matrixd &input_feats, unsigned frame) const
{
	unsigned num_frames = input_feats.size(),
		feat_dim = input_feats[0].size();
	std::vector<double> output_frame;
	output_frame.resize(feat_dim * (num_blocks_ + 1));
	KtuMath<double>::zeros(output_frame.data(), output_frame.size());

	// The original features
	KtuMath<double>::add(output_frame.data(), input_feats[frame].data(), output_frame.data(), feat_dim);

	// Concatenate the delta-blocks. Each block is block_shift
	// (usually 3) frames apart.
	for (int i = 0; i < num_blocks_; i++) {
		int max_offset = (scales_.size() - 1) / 2;
		double* output = output_frame.data() + (i + 1) * feat_dim;
		for (int j = -max_offset; j <= max_offset; j++) {
			int offset_frame = KtuMath<int>::clamp(frame + j + i * block_shift_, 0, num_frames - 1);
			double scale = scales_[j + max_offset];
			if (scale != 0.0) {
				const double* input = input_feats[offset_frame].data();
				for (size_t n = 0; n < feat_dim; n++)
					output[n] += input[n] * scale;
			}
		}
	}

	return output_frame;
}

KgShiftedDelta::matrixd KgShiftedDelta::compute(const matrixd &input_feats) const
{
	matrixd output_features(input_feats.size());
	for (unsigned r = 0; r < input_feats.size(); r++) 
		output_features[r] = process(input_feats, r);

	return output_features;
}