#include "KgDelta.h"
#include "KtuMath.h"
#include <assert.h>


KgDelta::KgDelta(unsigned order, unsigned window)
{
	assert(window != 0);  

	scales_.resize(order + 1);
	scales_[0].resize(1);
	scales_[0][0] = 1.0;  // trivial window for 0th order delta [i.e. baseline feats]

	for (unsigned i = 1; i <= order; i++) {
		std::vector<double> &prev_scales = scales_[i - 1],
			&cur_scales = scales_[i];

		int prev_offset = (static_cast<int>(prev_scales.size() - 1)) / 2,
			cur_offset = prev_offset + window;
		cur_scales.resize(prev_scales.size() + 2 * window); 
		KtuMath<double>::zeros(cur_scales.data(), static_cast<unsigned>(cur_scales.size())); 

		double normalizer = 0.0;
		int j = -static_cast<int>(window);
		for (; j <= static_cast<int>(window); j++) {
			normalizer += j*j;
			for (int k = -prev_offset; k <= prev_offset; k++) {
				cur_scales[j + k + cur_offset] +=
					static_cast<double>(j) * prev_scales[k + prev_offset];
			}
		}
		KtuMath<double>::scale(cur_scales.data(), 
			static_cast<unsigned>(cur_scales.size()), 1.0 / normalizer);
	}
}


std::vector<double> KgDelta::process(const matrixd &input_feats, unsigned frame) const
{
	assert(frame < input_feats.size());

	unsigned num_frames = static_cast<unsigned>(input_feats.size()),
		feat_dim = static_cast<unsigned>(input_feats[0].size());
	std::vector<double> output_frame;
	output_frame.resize(feat_dim * scales_.size());
	KtuMath<double>::zeros(output_frame.data(), static_cast<unsigned>(output_frame.size()));
	for (unsigned i = 0; i < scales_.size(); i++) {
		const std::vector<double> &scales = scales_[i];
		int max_offset = (static_cast<int>(scales.size()) - 1) / 2;
		double* output = output_frame.data() + i*feat_dim;
		for (int j = -max_offset; j <= max_offset; j++) {
			int offset_frame = KtuMath<int>::clamp(frame + j, 0, num_frames - 1);
			double scale = scales[j + max_offset];
			if (scale != 0.0) {
				const double* input = input_feats[offset_frame].data();
				for (unsigned n = 0; n < feat_dim; n++)
					output[n] += input[n] * scale;
			}
		}
	}

	return output_frame;
}

KgDelta::matrixd KgDelta::compute(const matrixd &input_feats) const
{
	matrixd output_features(input_feats.size());
	for (unsigned r = 0; r < input_feats.size(); r++)
		output_features[r] = process(input_feats, r);

	return output_features;
}