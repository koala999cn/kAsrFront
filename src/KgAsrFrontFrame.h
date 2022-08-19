#pragma once
#include <functional>
#include <vector>
#include "capture/KcVoicePicker.h"


// 1. ����json�����ļ����Զ�����ASRǰ����ˮ�߿��
// 2. ����voice-picker��feat-pipeline

class KgAsrFrontFrame
{
public:

	using feat_vector = std::vector<double>;
	using feat_matrix = std::vector<feat_vector>;
	using voice_handler = typename KcVoicePicker::voice_handler;


	// @configFilePath: json��ʽ�������ļ�
	KgAsrFrontFrame(const char* jsonPath);
	~KgAsrFrontFrame();

	bool run(std::function<void(const feat_matrix& feats)> h);

	void stop();

	// ���������ά��
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
