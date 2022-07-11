#include "KgAsrFrontFrame.h"
#include "nlohmann/json.hpp"
#include "capture/KcAudioDevice.h"
#include <fstream>
#include <list>
#include <string>


namespace kPrivate
{
	static void* config(const nlohmann::json& jobj);
}


KgAsrFrontFrame::KgAsrFrontFrame(const char* configFilePath)
	: dptr_(nullptr)
{
	std::ifstream ifs(configFilePath);
	if (ifs) {
		nlohmann::json jobj;
		ifs >> jobj;
		dptr_ = kPrivate::config(jobj);
	}
}


KgAsrFrontFrame::~KgAsrFrontFrame()
{

}


bool KgAsrFrontFrame::run()
{
	stop();

	return true;
}


void KgAsrFrontFrame::stop()
{

}


namespace kPrivate
{
	static KcAudioDevice* configInput(const nlohmann::json& jobj)
	{
		std::unique_ptr<KcAudioDevice> device;
		if (jobj.contains("input") && jobj["input"].is_object()) {
			device = std::make_unique<KcAudioDevice>();
			auto input = jobj["input"];
			
			unsigned deviceId = -1; // default input device
			if (input.contains("device")) {
				if (input["device"].is_number_integer())
					deviceId = input["device"].get<int>();
				else if (input["device"].is_string())
					deviceId = device->getDevice(input["device"].get<std::string>());
			}
			if (deviceId == -1) deviceId = device->defaultInput();

			unsigned sampleRate = 16000;
			if (input.contains("sampleRate") && input["sampleRate"].is_number())
				sampleRate = input["sampleRate"].get<unsigned>();

			unsigned frames = sampleRate * 0.05; // 50ms
			if (input.contains("frameTime") && input["frameTime"].is_number())
				frames = sampleRate * input["frameTime"].get<double>();

			KcAudioDevice::KpStreamParameters iparam;
			iparam.channels = 1; // always mono
			iparam.deviceId = deviceId;
			if (!device->open(nullptr, &iparam, KcAudioDevice::k_float64, sampleRate, frames))
				device.reset();
		}

		return device.release();
	}

	static void* config(const nlohmann::json& jobj)
	{
		configInput(jobj);
		//auto pipeline = configPipeline(jobj);
		//for (auto step : pipeline)
		//	configStep(jobj, step);

		return 0;
	}
}