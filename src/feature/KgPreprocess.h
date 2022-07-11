#pragma once
#include <vector>
#include <functional>


class KgPreprocess
{
public:

	struct KpOption
	{
		double frameLength; // Frame length in seconds (default = 0.025)
		double frameShift; // Frame shift in seconds (float, default = 0.01)

		bool dither; // Dithering constant (0.0 means no dither). If you turn this off, you should set the --energy-floor option, e.g. to 1.0 or 0.1 (float, default = 1)
		
		bool removeDcOffset; // Subtract mean from waveform on each frame (default = true)

		double preemphasis; // Coefficient for use in signal preemphasis (default = 0.97)
		
		bool useEnergy;
		bool rawEnergy; // If true, compute energy before preemphasisand windowing (default = true)
		double energyFloor;

		int windowType;
		double windowArg;
	};

	using frame_handler = std::function<bool(double* frame, double energy)>;

	KgPreprocess(const KpOption& option, frame_handler handler);

	~KgPreprocess();

	void process(const double* buf, unsigned len) const;

	void flush() const;


private:

	void processOneFrame_(const double* frame) const;

private:
	KpOption opts_;
	frame_handler handler_;
	void* dptr_;
};

