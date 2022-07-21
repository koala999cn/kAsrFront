#pragma once
#include <vector>


class KgDelta
{
public:

	typedef std::vector<std::vector<double>> matrixd;

	// order - Order of delta computation (default = 2)
	// window - Parameter controlling window for delta computation.
	//          actual window size for each delta order is (1 + 2*window) (default = 2)
	//          the behavior at the edges is to replicate the first or last frame.
	KgDelta(unsigned order, unsigned window);

	std::vector<double> process(const matrixd &input_feats, unsigned frame) const;

	matrixd compute(const matrixd &input_feats) const;

	// odim = (order + 1) * idim

private:
	std::vector<std::vector<double>> scales_;
};

