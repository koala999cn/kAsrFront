#include "KgVoixenVad.h"
#include "voixen-vad/simplevad.h"
#include <assert.h>
#include <vector>


KgVoixenVad::KgVoixenVad(int sampleRate, int mode) : sampleRate_(sampleRate)
{
	assert(sampleRate_ == 8000 || sampleRate_ == 16000 ||
		sampleRate_ == 32000 || sampleRate_ == 48000);

	size_t memSize;
	::vadAllocate(0, &memSize);
	obj_ = new char[memSize];
	vad_ = ::vadAllocate(obj_, &memSize);
	::vadInit(vad_t(vad_));
	::vadSetMode(vad_t(vad_), (vad_mode)mode);
}


KgVoixenVad::~KgVoixenVad()
{
	delete obj_;
}


bool KgVoixenVad::voiceActiveTest(const double* samples, size_t numSamples)
{
	static std::vector<float> data;
	data.resize(numSamples);
	for (size_t n = 0; n < numSamples; n++)
		data[n] = float(samples[n]);

	return ::vadProcessAudio(vad_t(vad_), sampleRate_, data.data(), numSamples) == VAD_EVENT_VOICE;
}