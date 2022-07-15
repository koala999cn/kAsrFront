#pragma once
#include <vector>
#include <functional>


class KgPreprocess
{
public:

	struct KpOptions
	{
		double sampleRate; // 输入数据的采样频率

		unsigned frameSize; // samples per frame (default = 0.025 * f)
		unsigned frameShift; // Frame shift in samples (default = 0.01 * f)
		
		double dither; // Dithering constant (0.0 means no dither). 
		
		bool removeDcOffset; // Subtract mean from waveform on each frame (default = true)

		double preemphasis; // 0.0 means no preemphasis, default = 0.97
		
		int windowType;
		double windowArg;

		int energyMode; // 0表示不计算信号的能量，1表示在preemphasis和windowing前计算信号能量，2表示预处理后计算信号能量. 
		                // E = sum(x[i]*x[i])
	};

	using frame_handler = std::function<bool(double* frame, double energy)>;

	KgPreprocess(const KpOptions& opts, frame_handler handler);

	~KgPreprocess();

	void process(const double* buf, unsigned len) const;

	void flush() const;

	// 只有输出维度，输入维度由用户提供
	unsigned dim() const {
		return opts_.frameShift; 
	}

	const KpOptions& options() const { return opts_; }

private:

	void processOneFrame_(const double* frame) const;

private:
	KpOptions opts_;
	frame_handler handler_;
	void* dptr_;
};

