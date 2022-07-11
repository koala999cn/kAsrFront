#pragma once
#include <vector>


/// 频谱分析功能类

class KgSpectrum
{
public:

	enum KeType
	{
		k_power, // |FFT|^2
		k_log,   // log(|FFT|^2)
		k_db,    // 10*log10(|FFT|^2)
		k_mag    // |FFT|
	};

	KgSpectrum(unsigned frameSize);
	~KgSpectrum();

	// 返回输入输出的规格
	std::pair<unsigned, unsigned> dim() const;

	void reset(unsigned frameSize);

	void porcess(double* data/*inout*/) const;

	KeType type() const { return type_; }
	void setType(KeType type) { type_ = type; }

	double floor() const { return floor_; }
	void setFloor(double f) { floor_ = f; }


private:
	void* rdft_;
	bool roundToPower2_;
	KeType type_; // 频谱类型
	double floor_; // 频谱底值，type_为k_log或k_db时有效
};
