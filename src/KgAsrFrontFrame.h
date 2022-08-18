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

	using pick_handler = std::function<void(std::vector<std::vector<double>>& feats)>;
	bool run(pick_handler h, std::function<void(void)> voice_notify);

	void stop();

	// 输出的特征维度
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
