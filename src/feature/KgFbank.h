#pragma once
#include <vector>


// 滤波器组分析：只适用于均匀采样数据
// 滤波器的各子带(bin)在目标频率尺度上始终是均匀划分的
 
class KgFbank
{
public:

	// Fbank类型
	enum KeType
	{
		k_linear,
		k_log,
		k_mel,
		k_bark,
		k_erb
	};

	// @lowFreq, @highFreq: 拟作fbank分析的频率范围(Hz)
	struct KpOptions
	{
		int type; // KeType
		unsigned numBins;
		double lowFreq;
		double highFreq;
	};

	// @sampleRate, @df: 拟输入频谱数据的规格
	// @opts: fbank变换的参数
	KgFbank(double sampleRate, double df, const KpOptions& opts);

	// 返回输入、输出的dim
	std::pair<unsigned, unsigned> dim() const;

	// @in: 输入频谱数据[0, nf], 共isize_个频点, x0ref = 0.5
	void process(const double* in, double* out);

	// 返回第bin个子带的中心频率(Hz)
	double fc(unsigned bin) const { return fc_[bin]; }

private:
	double toHertz_(double scale); // 将type频率尺度转换为hz
	double fromHertz_(double hz); // 将hz转换为type频率尺度
	static double calcFilterWeight_(double low, double high, double f);

private:
	KpOptions opts_;
	unsigned isize_; // 输入频谱数据的dim
	std::vector<unsigned> firstIdx_; // 各bin的起始点位置
	std::vector<double> fc_; // 各bin的中心点频率值(Hz)
	std::vector<std::vector<double>> weights_; // 各bin的权值数组，数组大小等于各bin的采样点数
};
