#pragma once
#include "KgPreprocess.h"
#include <memory>


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

	using spcetrum_handler = std::function<bool(double* spec)>;

	void process(const double* buf, unsigned len, spcetrum_handler sh) const;

	void flush(spcetrum_handler sh) const;

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
};
