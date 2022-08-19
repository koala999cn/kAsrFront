#pragma once
#include <functional>
#include <vector>
#include "capture/KcVoicePicker.h"


// 1. 根据json配置文件，自动构建ASR前端流水线框架
// 2. 链接voice-picker和feat-pipeline

class KgAsrFrontFrame
{
public:

	using feat_vector = std::vector<double>;
	using feat_matrix = std::vector<feat_vector>;
	using voice_handler = typename KcVoicePicker::voice_handler;


	// @configFilePath: json格式的配置文件
	KgAsrFrontFrame(const char* jsonPath);
	~KgAsrFrontFrame();

	bool run(std::function<void(const feat_matrix& feats)> h);

	void stop();

	// 输出的特征维度
	unsigned odim() const;

	// for debug
	void process(const double* buf, unsigned frames) const;
	
	// for debug
	auto& features() const { return feats_; }

	// for debug
	void setVoiceHandler(voice_handler h) {
		vhandler_ = h;
	}

	double sampleRate() const;

	double frameTime() const;
	unsigned frameSize() const;

	double shiftTime() const;
	unsigned shiftSize() const;

private:
	void* dptr_;
	bool tracking_;
	feat_matrix feats_;
	voice_handler vhandler_;
};
