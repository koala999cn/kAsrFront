#pragma once
#include <functional>


// �����Զ�ʰȡ

class KcVoicePicker
{
public:

	enum class KeVoiceEvent
	{
		k_voice_picked, // ��ȡ��һ������
		k_voice_discard, // ������һ��������ʱ�����㣩
		k_voice_frame, // voice֡
		k_unvoice_frame // unvoice֡
	};

	struct KpEventData
	{
		double* inbuf;
		unsigned frames;
		double streamTime;
	};

	using voice_handler = std::function<bool(KeVoiceEvent, const KpEventData&)>;

	// @minVoiceDuration: ��̳���ʱ�䡣����ʱ��С�ڸ�ֵ�����Σ�����k_voice_discard�¼�
	// @maxWaitTime: ���ȴ�ʱ�䡣��������ֵδ��⵽voice֡����ȡǰ�����Σ�����k_voice_picked�¼�
	KcVoicePicker(double minVoiceDuration = 0.3, double maxWaitTime = 0.5);

	virtual ~KcVoicePicker(void);

	// @vadMode: ȡֵ0-3
	//  - 0: normalģʽ
	//  - 1: low-bitrateģʽ
	//  - 2: aggressiveģʽ
	//  - 3: very aggressiveģʽ
	bool listen(unsigned sampleRate, unsigned device, float frameTime, int vadMode, voice_handler handler);

	void pause();

	void goon();

	void stop();

private:
	void* dptr_;
};

