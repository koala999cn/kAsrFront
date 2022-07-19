#pragma once
#include <vector>
#include <functional>


class KgPreprocess
{
public:

	enum KeEnergyMode
	{
		k_energy_none,
		k_energy_raw,
		k_energy_post
	};

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

		// TODO: kaldi的snip-edges暂不支持，始终默认为true
	};


	KgPreprocess(const KpOptions& opts);

	~KgPreprocess();

	using frame_handler = std::function<bool(double* frame, double energy)>;

	void process(const double* buf, unsigned len, frame_handler fh) const;

	void flush(frame_handler fh) const;

	// 只有输出维度，输入维度由用户提供
	unsigned odim() const {
		return opts_.frameSize; 
	}

	const KpOptions& options() const { return opts_; }

private:

	double processOneFrame_(const double* in, double* out) const;

private:
	KpOptions opts_;
	void* dptr_;
};

