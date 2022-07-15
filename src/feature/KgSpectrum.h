#pragma once
#include <vector>


/// 频谱分析功能类

class KgSpectrum
{
public:

	// 支持的频谱类型
	enum KeType
	{
		k_power, // |FFT|^2, praat模式
		k_log,   // log(|FFT|^2), kaldi模式
		k_db,    // 10*log10(|FFT|^2)
		k_mag    // |FFT|
	};

	// 频谱归一化模式
	enum KeNormMode
	{
		k_norm_none, // 不作归一化，节省计算量
		k_norm_default, // 用时域信号长度N作归一化(KgRdft可控参数)
		k_norm_praat, // 用采样频率F作归一化(兼容praat)
		k_norm_kaldi // 用int16最大值作归一化(兼容kaldi)，非规范操作，慎用
	};

	// @frameSize: 输入数据的长度
	KgSpectrum(unsigned frameSize, double sampleRate, KeNormMode norm);
	~KgSpectrum();

	// 返回输入输出的规格
	std::pair<unsigned, unsigned> dim() const;

	void reset(unsigned frameSize);

	void porcess(double* data/*inout*/) const;

	int type() const { return type_; }
	void setType(KeType type) { type_ = type; }

	int normMode() const { return norm_; }

	double sampleRate() const { return f_; }

private:
	void* rdft_;
	KeType type_;
	const KeNormMode norm_;
	const double f_; // 采样频率
};
