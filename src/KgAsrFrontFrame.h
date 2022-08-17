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

	bool run(std::function<void(std::vector<std::vector<double>>& feats)> h);

	void stop();

	// ���������ά��
	unsigned odim() const;

	void process(const double* buf, unsigned frames) const;
	
	auto& features() const { return feats_; }

private:
	void* input_;
	void* pipe_;
	bool tracking_;
	std::vector<std::vector<double>> feats_;
};
