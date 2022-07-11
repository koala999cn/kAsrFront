#include "capture/KcVoicePicker.h"
#include <thread>
#include <conio.h>


int main(int argc, char const* argv[])
{
	KcVoicePicker picker;
	if(!picker.listen(16000, -1, 0.05, 3,
		[](KcVoicePicker::KeVoiceEvent id, const KcVoicePicker::KpEventData& data) {
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
