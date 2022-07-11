#pragma once
#include <functional>


// 语音自动拾取

class KcVoicePicker
{
public:

	enum class KeVoiceEvent
	{
		k_voice_picked, // 获取了一段语音
		k_voice_discard, // 丢弃了一段语音（时长不足）
		k_voice_frame, // voice帧
		k_unvoice_frame // unvoice帧
	};

	struct KpEventData
	{
		double* inbuf;
		unsigned frames;
		double streamTime;
	};

	using voice_handler = std::function<bool(KeVoiceEvent, const KpEventData&)>;

	// @minVoiceDuration: 最短持续时间。丢弃时长小于该值的音段，发送k_voice_discard事件
	// @maxWaitTime: 最大等待时间。若超过该值未检测到voice帧，截取前序音段，发送k_voice_picked事件
	KcVoicePicker(double minVoiceDuration = 0.3, double maxWaitTime = 0.5);

	virtual ~KcVoicePicker(void);

	// @vadMode: 取值0-3
	//  - 0: normal模式
	//  - 1: low-bitrate模式
	//  - 2: aggressive模式
	//  - 3: very aggressive模式
	bool listen(unsigned sampleRate, unsigned device, float frameTime, int vadMode, voice_handler handler);

	void pause();

	void goon();

	void stop();

private:
	void* dptr_;
};

