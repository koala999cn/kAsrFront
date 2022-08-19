#include "KcVoicePicker.h"
#include "KcAudioDevice.h"
#include "KgVoixenVad.h"
#include <assert.h>


namespace kPrivate
{
	struct KpVoicePickerImpl_
	{
		KcVoicePicker::KpOptions opts;
		std::unique_ptr<KcAudioDevice> device;
		std::unique_ptr<KgVoixenVad> vad;

		double voicedTime; // �׸�voice֡��ʱ��
		std::vector<double> voicedData;

		unsigned maxAheadPadding_; // ǰ׺unvoice���������
		std::vector<double> aheadPadding_;

		double unvoicedTime; // ����unvoice֡����ʼʱ��
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
				dptr_->unvoicedTime = streamEndTime; // ��Ϊ��һ֡ʱ��

				if (dptr_->voicedData.empty())// �׸�voiced֡��ͬ��ʱ��
					dptr_->voicedTime = streamTime;

				dptr_->voicedData.insert(dptr_->voicedData.end(), input, input + frames);
			}
			else {
				if (!dptr_->voicedData.empty()) { // picking״̬�µ�unvoiced֡

					dptr_->voicedData.insert(dptr_->voicedData.end(), input, input + frames); // ѹ��voiced����ջ����Ϊpicking���ݵ�һ����

					if (streamEndTime - dptr_->unvoicedTime >= dptr_->opts.maxWaitTime) { // �㹻ʱ����unvoiced

						auto duration = dptr_->unvoicedTime - dptr_->voicedTime;

						if (duration >= dptr_->opts.minVoiceDuration)
							// ���aheadPadding
							dptr_->voicedData.insert(dptr_->voicedData.begin(), 
								dptr_->aheadPadding_.begin(), dptr_->aheadPadding_.end());

						userData.inbuf = dptr_->voicedData.data();
						userData.frames = static_cast<unsigned>(dptr_->voicedData.size());
						userData.streamTime = dptr_->voicedTime;

						if (!handler_(duration >= dptr_->opts.minVoiceDuration ?
							KcVoicePicker::KeVoiceEvent::k_voice_picked 
							: KcVoicePicker::KeVoiceEvent::k_voice_discard, userData))
							return false;

						// ����״̬��unvoicedTime���ö�
						dptr_->voicedData.clear();
						dptr_->aheadPadding_.clear();
					}
				}
				else {
					// ����unvoiced״̬������aheadPadding
					dptr_->aheadPadding_.insert(dptr_->aheadPadding_.end(),
						input, input + frames);
					if (dptr_->aheadPadding_.size() > dptr_->maxAheadPadding_) {
						auto extra = dptr_->aheadPadding_.size() - dptr_->maxAheadPadding_;
						dptr_->aheadPadding_.erase(dptr_->aheadPadding_.begin(),
							dptr_->aheadPadding_.begin() + extra);
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


KcVoicePicker::KcVoicePicker(const KpOptions& opts)
{
	auto sampleRate = opts.sampleRate;
	if (sampleRate != 8000
		&& sampleRate != 16000
		&& sampleRate != 32000
		&& sampleRate != 48000)
		sampleRate = 16000;

	auto d = new kPrivate::KpVoicePickerImpl_;
	d->device = std::make_unique<KcAudioDevice>();
	d->opts = opts;
	d->voicedTime = 0;
	d->unvoicedTime = 0;
	d->maxAheadPadding_ = opts.aheadPaddingTime * sampleRate;
	dptr_ = d;

	auto device = opts.deviceId;
	if (device == -1)
		device = d->device->defaultInput();
	auto frames = unsigned(sampleRate * opts.frameTime);

	KcAudioDevice::KpStreamParameters iparam;
	iparam.channels = 1; // always mono
	iparam.deviceId = device;
	d->device->open(nullptr, &iparam, KcAudioDevice::k_float64, sampleRate, frames);
	d->vad = std::make_unique<KgVoixenVad>(sampleRate, opts.vadMode);
}

KcVoicePicker::~KcVoicePicker(void)
{
	delete (kPrivate::KpVoicePickerImpl_*)dptr_;
}


bool KcVoicePicker::run(voice_handler handler)
{
	auto d = (kPrivate::KpVoicePickerImpl_*)dptr_;

	if (!d->device->opened())
		return false;

	if(d->device->running())
	    stop();

	d->device->remove<kPrivate::KcPickObserver>();
	d->device->pushBack(std::make_shared<kPrivate::KcPickObserver>(d, handler));

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
	if (d->device->running())
	    d->device->stop(true);
	d->device->close();
	d->device->remove<kPrivate::KcPickObserver>();
}


const KcVoicePicker::KpOptions& KcVoicePicker::options() const
{
	auto d = (kPrivate::KpVoicePickerImpl_*)dptr_;
	return d->opts;
}