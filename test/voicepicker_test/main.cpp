#include "capture/KcVoicePicker.h"
#include <thread>
#include <conio.h>


int main(int argc, char const* argv[])
{
	KcVoicePicker::KpOptions opts;
	opts.deviceId = -1;
	opts.sampleRate = 16000;
	opts.vadMode = 3;
	opts.frameTime = 0.05f;
	opts.minVoiceDuration = 0.3;
	opts.maxWaitTime = 0.5;
	opts.aheadPaddingTime = 0.05;
	KcVoicePicker picker(opts);
	if(!picker.run([](KcVoicePicker::KeVoiceEvent id, const KcVoicePicker::KpEventData& data) {
			if (id == KcVoicePicker::KeVoiceEvent::k_voice_frame)
				printf("-");
			else if (id == KcVoicePicker::KeVoiceEvent::k_voice_picked)
				printf(">.\n");
			else if (id == KcVoicePicker::KeVoiceEvent::k_voice_discard)
				printf("<.\n");
		
		    return true;
		}))
		return -1;

	printf("start listening...\n");
	while (!_kbhit())
		std::this_thread::yield();

	return 0;
}
