#pragma once
#include <functional>
#include <vector>


// 1. ����json�����ļ����Զ�����ASRǰ����ˮ�߿��
// 2. ����voice-picker��feat-pipeline

class KgAsrFrontFrame
{
public:

	// @configFilePath: json��ʽ�������ļ�
	KgAsrFrontFrame(const char* jsonPath);
	~KgAsrFrontFrame();

	using pick_handler = std::function<void(std::vector<std::vector<double>>& feats)>;
	bool run(pick_handler h, std::function<void(void)> voice_notify);

	void stop();

	// ���������ά��
	unsigned odim() const;

	// for debug
	void process(const double* buf, unsigned frames) const;
	
	// for debug
	auto& features() const { return feats_; }

private:
	void* input_;
	void* pipe_;
	bool tracking_;
	std::vector<std::vector<double>> feats_;
};
