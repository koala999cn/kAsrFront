#pragma once


// ASR前端框架

class KgAsrFrontFrame
{
public:

	// @configFilePath: json格式的配置文件
	KgAsrFrontFrame(const char* configFilePath);
	~KgAsrFrontFrame();

	bool run();

	void stop();

private:
	void* dptr_;
};
