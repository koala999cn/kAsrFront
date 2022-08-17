#pragma once
#include <functional>
#include <vector>


// 1. 根据json配置文件，自动构建ASR前端流水线框架
// 2. 链接voice-picker和feat-pipeline

class KgAsrFrontFrame
{
public:

	// @configFilePath: json格式的配置文件
	KgAsrFrontFrame(const char* jsonPath);
	~KgAsrFrontFrame();

	bool run(std::function<void(std::vector<std::vector<double>>& feats)> h);

	void stop();

	// 输出的特征维度
	unsigned odim() const;

	void process(const double* buf, unsigned frames) const;
	
	auto& features() const { return feats_; }

private:
	void* input_;
	void* pipe_;
	bool tracking_;
	std::vector<std::vector<double>> feats_;
};
