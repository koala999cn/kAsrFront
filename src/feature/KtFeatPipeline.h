#pragma once
#include "KgPreprocess.h"
#include <memory>
#include <tuple>
#include <assert.h>


template<typename... OP>
class KtFeatPipeline
{
public:

	KtFeatPipeline(const KgPreprocess::KpOptions& opts, OP&&... ops) 
	    : ops_(std::make_tuple(std::forward<OP>(ops)...)) {
		prep_ = std::make_unique<KgPreprocess>(opts);
		prep_->setHandler([this](double* frame, double energy) {
			std::vector<double> out(odim());
			doPipeline_(frame, out.data());
			if (prep_->options().energyMode != KgPreprocess::k_energy_none)
				out[0] = energy; // TODO: fix energy
			handler_(out.data());
			});
	}

	void setHandler(std::function<void(double* spec)> h) {
		handler_ = h;
	}

	void process(const double* buf, unsigned len) const {
		prep_->process(buf, len);
	}

	void flush() const {
		prep_->flush();
	}

	// 只有输出维度，输入维度由用户提供
	unsigned odim() const {
		return std::get<std::tuple_size_v<std::tuple<OP...>> - 1>(ops_).odim();
	}


private:

	template<int N>
	void doPipeline_(const double* buf, double* out) const {
		auto& op = std::get<N - 1>(ops_);
		assert(op.idim() == std::get<N - 2>(ops_).odim());
		std::vector<double> temp(op.idim());
		doPipeline_<N - 1>(buf, temp.data());
		op.process(temp.data(), out);
	}

	template<>
	void doPipeline_<1>(const double* buf, double* out) const {
		std::get<0>(ops_).process(buf, out);
	}

	void doPipeline_(const double* buf, double* out) const {
		doPipeline_<std::tuple_size_v<std::tuple<OP...>>>(buf, out);
	}

private:
	std::unique_ptr<KgPreprocess> prep_;
	std::tuple<OP...> ops_;
	std::function<void(double* spec)> handler_;
};

/*
class KgSpectrogram
{
public:

	struct KpOptions : public KgPreprocess::KpOptions
	{
		int type; // 频谱类型, 参见KgSpectrum::KeType
		int norm; // 归一化模式，参见KgSpectrum::KeNormMode
		bool roundToPower2;
		double energyFloor; // 当信号energy小于该值时，用该值替代信号energy值，与norm高度相关
	};

	
	KgSpectrogram(const KpOptions& opts);

	~KgSpectrogram();

	void setHandler(std::function<void(double* spec)> h);

	void process(const double* buf, unsigned len) const;

	void flush() const;

	// 只有输出维度，输入维度由用户提供
	unsigned odim() const;

private:

	void processOneFrame_(double* in, double* out) const;

	// 根据频谱类型和归一化模式，修正能量值
	double fixEnergy_(double energy) const;

private:
	double energyFloor_;
	std::unique_ptr<KgPreprocess> prep_;
	void* dptr_;
	std::function<void(double* spec)> handler_;
};
*/