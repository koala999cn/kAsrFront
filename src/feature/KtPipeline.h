#pragma once
#include <tuple>
#include <assert.h>


// 流水线操作
// 每个OP算子具有3个成员方法：idim, odim, process
// KtPipeline本身也可作为OP加入到更上一级的pipeline

template<typename... OP>
class KtPipeline
{
public:

	KtPipeline(OP&&... ops)
		: ops_(std::make_tuple(std::forward<OP>(ops)...)) {
	}

	unsigned idim() const {
		return get<0>().idim();
	}

	unsigned odim() const {
		return get<size()-1>().odim();
	}

	void process(const double* in, double* out) const {
		doPipeline<size()>(in, out);
	}


	// pipeline元素个数
	constexpr static auto size() {
		return std::tuple_size_v<std::tuple<OP...>>;
	}

	template<int I>
	auto& get() { return std::get<I>(ops_); }

	template<int I>
	auto& get() const { return std::get<I>(ops_); }


	template<int N>
	void doPipeline(const double* buf, double* out) const {
		auto& op = get<N - 1>();
		assert(op.idim() == get<N - 2>().odim());
		std::vector<double> temp(op.idim());
		doPipeline<N - 1>(buf, temp.data());
		op.process(temp.data(), out);
	}

	template<>
	void doPipeline<1>(const double* buf, double* out) const {
		get<0>().process(buf, out);
	}

private:
	std::tuple<OP...> ops_;
};
