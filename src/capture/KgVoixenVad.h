#pragma once

class KgVoixenVad
{
public:
	// @mode: ȡֵ0-3
	//  - 0: normalģʽ
	//  - 1: low-bitrateģʽ
	//  - 2: aggressiveģʽ
	//  - 3: very aggressiveģʽ
	KgVoixenVad(int sampleRate, int mode = 0);
	~KgVoixenVad();


	bool voiceActiveTest(const double* samples, size_t numSamples);

private:
	void* obj_;
	void* vad_;
	int sampleRate_;
};

