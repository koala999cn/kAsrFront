#include "KcVoicePicker.h"
#include "KcAudioDevice.h"
#include "KgVoixenVad.h"
#include <assert.h>


namespace kPrivate
{
	struct KpVoicePickerImpl_
	{
		std::unique_ptr<KcAudioDevice> device;
		std::unique_ptr<KgVoixenVad> vad;

		double minVoiceDuration; // 最小语音时间长度，小于该长度的语音被抛弃，不触发事件
		double maxWaitTime; // 非语音时间超过该值后，认为当前语音已结束，触发事件

		double voicedTime;
		std::vector<double> voicedData;

		double unvoicedTime;
	};

	class KcPickObserver : public KcAudioDevice::observer_type
	{
	public:

		KcPickObserver(KpVoicePickerImpl_* dptr, KcVoicePicker::voice_handler handler) 
			: dptr_(dptr), handler_(handler) {}

		bool update(void*, void* inputBuffer, unsigned frames, double streamTime) override {
			auto input = (double*)inputBuffer;

			bool active = dptr_->vad->voiceActiveTest(input, frames);
			KcVoicePicker::KpEventData userData{ input, frames, streamTime };
			if (!handler_(active ? KcVoicePicker::KeVoiceEvent::k_voice_frame 
				: KcVoicePicker::KeVoiceEvent::k_unvoice_frame, userData))
				return false;

			double streamEndTime = streamTime + double(frames) / dptr_->device->sampleRate();

			if (active) {
				dptr_->unvoicedTime = streamEndTime; // 置为下一帧时间

				if (dptr_->voicedData.empty())// 首个voiced帧，同步时间
					dptr_->voicedTime = streamTime;

				dptr_->voicedData.insert(dptr_->voicedData.end(), input, input + frames);
			}
			else {
				if (!dptr_->voicedData.empty()) { // 处理unvoiced帧

					dptr_->voicedData.insert(dptr_->voicedData.end(), input, input + frames); // 仍须压入voiced数据栈

					if (streamEndTime - dptr_->unvoicedTime >= dptr_->maxWaitTime) { // 足够时长的unvoiced

						auto duration = dptr_->unvoicedTime - dptr_->voicedTime;
						userData.inbuf = dptr_->voicedData.data();
						userData.frames = static_cast<unsigned>(duration * dptr_->device->sampleRate() + 0.5);
						userData.streamTime = dptr_->voicedTime;
						if (!handler_(duration >= dptr_->minVoiceDuration ?
							KcVoicePicker::KeVoiceEvent::k_voice_picked 
							: KcVoicePicker::KeVoiceEvent::k_voice_discard, userData))
							return false;

						// 重置状态，unvoicedTime不用动
						dptr_->voicedData.clear();
					}
				}
			}

			return true;
		}

	private:
		KpVoicePickerImpl_* dptr_;
		KcVoicePicker::voice_handler handler_;
	};
}


KcVoicePicker::KcVoicePicker(double minVoiceDuration, double maxWaitTime)
{
	auto d = new kPrivate::KpVoicePickerImpl_;
	d->device = std::make_unique<KcAudioDevice>();
	d->minVoiceDuration = minVoiceDuration;
	d->maxWaitTime = maxWaitTime;
	d->voicedTime = 0;
	d->unvoicedTime = 0;
	dptr_ = d;
}

KcVoicePicker::~KcVoicePicker(void)
{
	delete (kPrivate::KpVoicePickerImpl_*)dptr_;
}


bool KcVoicePicker::listen(unsigned sampleRate, unsigned device, float frameTime, int vadMode, voice_handler handler)
{
	if (sampleRate != 8000
		&& sampleRate != 16000
		&& sampleRate != 32000
		&& sampleRate != 48000)
		return false;

	auto d = (kPrivate::KpVoicePickerImpl_*)dptr_;

	if(d->device->running())
	    stop();

	if (device == -1)
		device = d->device->defaultInput();
	unsigned frames = sampleRate * frameTime;

	KcAudioDevice::KpStreamParameters iparam;
	iparam.channels = 1; // always mono
	iparam.deviceId = device;
	if (!d->device->open(nullptr, &iparam, KcAudioDevice::k_float64, sampleRate, frames))
		return false;

	d->device->pushBack(std::make_shared<kPrivate::KcPickObserver>((kPrivate::KpVoicePickerImpl_*)dptr_, handler));

	d->vad = std::make_unique<KgVoixenVad>(sampleRate, vadMode);

	return d->device->start();
}


void KcVoicePicker::pause()
{
	auto d = (kPrivate::KpVoicePickerImpl_*)dptr_;
	d->device->stop(true);
}


void KcVoicePicker::goon()
{
	auto d = (kPrivate::KpVoicePickerImpl_*)dptr_;
	d->device->start();
}


void KcVoicePicker::stop()
{
	auto d = (kPrivate::KpVoicePickerImpl_*)dptr_;
	d->device->stop(true);
	d->device->close();
	d->device->remove<kPrivate::KcPickObserver>();
}
