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

	// @vadMode: ȡֵ0-3
	//  - 0: normalģʽ
	//  - 1: low-bitrateģʽ
	//  - 2: aggressiveģʽ
	//  - 3: very aggressiveģʽ
	// @minVoiceDuration: ��̳���ʱ�䡣����ʱ��С�ڸ�ֵ�����Σ�����k_voice_discard�¼�
	// @maxWaitTime: ���ȴ�ʱ�䡣��������ֵδ��⵽voice֡����ȡǰ�����Σ�����k_voice_picked�¼�
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

