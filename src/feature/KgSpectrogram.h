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

	using spcetrum_handler = std::function<bool(double* spec)>;

	KgSpectrogram(const KpOptions& opts, spcetrum_handler h);

	~KgSpectrogram();

	void process(const double* buf, unsigned len) const;

	void flush() const;

	// 只有输出维度，输入维度由用户提供
	unsigned dim() const;

private:

	// 根据频谱类型和归一化模式，修正能量值
	double fixEnergy_(double energy) const;

private:
	unsigned fftFramingSize_; // 用来作fft的frame大小，roundToPower2为假时等于输入的frameSize，为真时等于ceilPower2(frameSize)
	double energyFloor_;
	std::unique_ptr<KgPreprocess> prep_;
	void* dptr_;
};
