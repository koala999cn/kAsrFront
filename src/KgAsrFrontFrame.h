#pragma once


// ASRǰ�˿��

class KgAsrFrontFrame
{
public:

	// @configFilePath: json��ʽ�������ļ�
	KgAsrFrontFrame(const char* configFilePath);
	~KgAsrFrontFrame();

	bool run();

	void stop();

private:
	void* dptr_;
};
