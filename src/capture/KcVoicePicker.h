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

	// @vadMode: 取值0-3
	//  - 0: normal模式
	//  - 1: low-bitrate模式
	//  - 2: aggressive模式
	//  - 3: very aggressive模式
	// @minVoiceDuration: 最短持续时间。丢弃时长小于该值的音段，发送k_voice_discard事件
	// @maxWaitTime: 最大等待时间。若超过该值未检测到voice帧，截取前序音段，发送k_voice_picked事件
	struct KpOptions
	{
		unsigned deviceId;
		unsigned sampleRate;
		float frameTime;
		int vadMode;
		double minVoiceDuration; // default 0.3
		double maxWaitTime; // default 0.5
	};

	KcVoicePicker(const KpOptions& opts);

	virtual ~KcVoicePicker(void);

	bool run(voice_handler handler);

	void pause();

	void goon();

	void stop();

	const KpOptions& options() const;

private:
	void* dptr_;
};

