#pragma once
#include <vector>

// 实现kaldi的delta算法

class KgDelta
{
public:

	typedef std::vector<std::vector<double>> matrixd;

	struct KpOptions
	{
		unsigned idim;
		unsigned order;  // Order of delta computation (default = 2)
		unsigned window;  // Parameter controlling window for delta computation.
	                      // actual window size for each delta order is (1 + 2*window) (default = 2)
	                      // the behavior at the edges is to replicate the first or last frame.
	};

	KgDelta(const KpOptions& opts);

	std::vector<double> process(const matrixd &input_feats, unsigned frame) const;

	matrixd compute(const matrixd &input_feats) const;

	// 返回输入、输出的dim
	unsigned idim() const { return opts_.idim; }
	unsigned odim() const {	return (opts_.order + 1) * idim(); }

	void process(const double* in, double* out) const;

private:
	KpOptions opts_;
	std::vector<std::vector<double>> scales_; 
	mutable std::vector<std::vector<double>> binput_; // buffered inputs
};

