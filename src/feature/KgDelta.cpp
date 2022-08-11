#include "KgDelta.h"
#include "KtuMath.h"
#include <assert.h>


KgDelta::KgDelta(const KpOptions& opts)
	: opts_(opts)
{
	assert(opts.window != 0);

	scales_.resize(opts.order + 1);
	scales_[0].resize(1);
	scales_[0][0] = 1.0;  // trivial window for 0th order delta [i.e. baseline feats]

	for (unsigned i = 1; i <= opts.order; i++) {
		std::vector<double> &prev_scales = scales_[i - 1],
			&cur_scales = scales_[i];

		int prev_offset = (static_cast<int>(prev_scales.size() - 1)) / 2,
			cur_offset = prev_offset + opts.window;
		cur_scales.resize(prev_scales.size() + 2 * opts.window);
		KtuMath<double>::zeros(cur_scales.data(), static_cast<unsigned>(cur_scales.size())); 

		double normalizer = 0.0;
		int j = -static_cast<int>(opts.window);
		for (; j <= static_cast<int>(opts.window); j++) {
			normalizer += j * j;
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


void KgDelta::process(const double* in, double* out) const
{
	auto context = opts_.order * opts_.window;
	binput_.push_back({ in, in + idim() });
	if (binput_.size() > 2 * context + 1)
		binput_.erase(binput_.begin());

	KtuMath<double>::zeros(out, odim());

	for (unsigned i = 0; i < scales_.size(); i++) {
		auto& scales = scales_[i];
		int max_offset = (static_cast<int>(scales.size()) - 1) / 2;
		double* p = out + i * idim();
		for (int j = -max_offset; j <= max_offset; j++) {
			auto offset_frame = KtuMath<int>::clamp(binput_.size() - 1 - context + j, 0, binput_.size() - 1);
			double scale = scales[j + max_offset];
			if (scale != 0.0) {
				const auto& in = binput_[offset_frame];
				for (unsigned n = 0; n < idim(); n++)
					p[n] += in[n] * scale;
			}
		}
	}
}