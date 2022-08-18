#include "KgAsrFrontFrame.h"
#include <fstream>
#include <list>
#include <string>
#include <assert.h>
#include "nlohmann/json.hpp"
#include "capture/KcVoicePicker.h"
#include "capture/KgWindowing.h"
#include "feature/KgPreprocess.h"
#include "feature/KgFbankPipe.h"
#include "feature/KgMfccPipe.h"
#include "feature/KgDelta.h"
#include "KvFeatPipeline.h"


namespace kPrivate
{
	static std::pair<KcVoicePicker*, KvFeatPipeline*> config(const nlohmann::json& jobj);
}


KgAsrFrontFrame::KgAsrFrontFrame(const char* jsonPath)
	: input_(nullptr), pipe_(nullptr)
{
	std::ifstream ifs(jsonPath);
	nlohmann::json jobj;
	try {
		ifs >> jobj;
	}
	catch (...) {
		;
	}

	auto r = kPrivate::config(jobj);
	input_ = r.first;
	pipe_ = r.second;

	r.second->setHandler([this](double* data) {
		feats_.push_back(std::vector<double>(data, data + odim()));
		});
}


KgAsrFrontFrame::~KgAsrFrontFrame()
{
	stop();
	delete (KcVoicePicker*)input_;
	delete (KvFeatPipeline*)pipe_;
}


bool KgAsrFrontFrame::run(pick_handler h, std::function<void(void)> voice_notify)
{
	auto input = (KcVoicePicker*)input_;
	tracking_ = false;

	return input->run([voice_notify, h, this](KcVoicePicker::KeVoiceEvent e, const KcVoicePicker::KpEventData& data) {
		if (e == KcVoicePicker::KeVoiceEvent::k_voice_discard) {
			feats_.clear();
			tracking_ = false;
		}
		else {
			if (e == KcVoicePicker::KeVoiceEvent::k_voice_frame) {
				tracking_ = true;
				voice_notify();
			}

			auto pipe = (KvFeatPipeline*)pipe_;

			if (tracking_)
				pipe->process(data.inbuf, data.frames);

			if (e == KcVoicePicker::KeVoiceEvent::k_voice_picked) {
				pipe->flush();
	
				KgDelta::KpOptions opts;
				opts.idim = odim();
				opts.order = 2;
				opts.window = 2;
				KgDelta delta(opts);
				auto feats = delta.compute(feats_);
				feats_.clear();
				tracking_ = false;

				h(feats);
			}
		}

		return true;
		});
}


void KgAsrFrontFrame::stop()
{
	if(input_)
	    ((KcVoicePicker*)input_)->stop();
}


void KgAsrFrontFrame::process(const double* buf, unsigned frames) const
{
	auto pipe = (KvFeatPipeline*)pipe_;
	pipe->process(buf, frames);
}


namespace kPrivate
{
	static KcVoicePicker* config_input(const nlohmann::json& jobj)
	{
		KcVoicePicker::KpOptions opts;
		opts.deviceId = -1; // default input device
		opts.sampleRate = 16000;
		opts.vadMode = 0;
		opts.minVoiceDuration = 0.3;
		opts.maxWaitTime = 0.5;
		opts.frameTime = 0.1f;

		if (jobj.contains("input") && jobj["input"].is_object()) {
			auto input = jobj["input"];
			if (input.contains("device")) {
				if (input["device"].is_number_integer())
					opts.deviceId = input["device"].get<int>();
				//else if (input["device"].is_string())
				//	opts.deviceId = device->getDevice(input["device"].get<std::string>());
			}

			if (input.contains("sample-rate") && input["sample-rate"].is_number())
				opts.sampleRate = input["sample-rate"].get<unsigned>();

			if (input.contains("frame-time") && input["frame-time"].is_number())
				opts.frameTime = input["frame-time"].get<float>();
		}

		return new KcVoicePicker(opts);
	}

	static KgPreprocess::KpOptions default_prep(double sampleRate)
	{
		KgPreprocess::KpOptions opts;
		opts.dither = false;
		opts.frameShift = decltype(opts.frameShift)(0.01 * sampleRate);
		opts.frameSize = decltype(opts.frameSize)(0.025 * sampleRate);
		opts.preemphasis = 0.97;
		opts.removeDcOffset = true;
		opts.sampleRate = sampleRate;
		opts.useEnergy = KgPreprocess::k_use_energy_raw;
		opts.windowType = KgWindowing::k_povey;
		opts.windowArg = 0.42;

		return opts;
	}

	static KgPreprocess::KpOptions config_prep(const nlohmann::json& jobj, double sampleRate)
	{
		auto opts = default_prep(sampleRate);

		if (jobj.contains("prep") && jobj["prep"].is_object()) {
			auto prep = jobj["prep"];

			if (prep.contains("frame-time") && prep["frame-time"].is_number()) {
				auto frameTime = prep["frame-time"].get<double>();
				opts.frameSize = decltype(opts.frameSize)(frameTime * sampleRate);
			}

			if (prep.contains("frame-shift") && prep["frame-shift"].is_number()) {
				auto frameShift = prep["frame-shift"].get<double>();
				opts.frameShift = decltype(opts.frameShift)(frameShift * sampleRate);
			}

			if (prep.contains("dither") && prep["dither"].is_number()) 
				opts.dither = prep["dither"].get<double>();

			if (prep.contains("preemphasis") && prep["preemphasis"].is_number())
				opts.preemphasis = prep["preemphasis"].get<double>();

			if (prep.contains("remove-dc") && prep["remove-dc"].is_boolean())
				opts.removeDcOffset = prep["remove-dc"].get<bool>();

			if (prep.contains("window") && prep["window"].is_string()) {
				auto indowType = KgWindowing::str2Type(prep["window"].get<std::string>().c_str());
				if (indowType != KgWindowing::k_unknown)
					opts.windowType = indowType;
			}

			if (prep.contains("window-arg") && prep["window-arg"].is_number())
				opts.windowArg = prep["window-arg"].get<double>();

			if (prep.contains("use-energy") && prep["use-energy"].is_string()) {
				auto useEnergy = prep["use-energy"].get<std::string>();
				if (::_stricmp(useEnergy.c_str(), "win") == 0)
					opts.useEnergy = KgPreprocess::k_use_energy_win;
				else if (::_stricmp(useEnergy.c_str(), "none") == 0)
					opts.useEnergy = KgPreprocess::k_use_energy_none;
			}
		}

		return opts;
	}

	static KgFbankPipe::KpOptions default_fbank()
	{
		KgFbankPipe::KpOptions opts;
		opts.bankNorm = false;
		opts.bankType = KgFbank::k_mel;
		opts.energyFloor = 1;
		opts.highFreq = 0;
		opts.lowFreq = 20;
		opts.numBanks = 23;
		opts.roundToPower2 = true;
		opts.specNorm = KgSpectrum::k_norm_kaldi;
		opts.specType = KgSpectrum::k_power;

		return opts;
	}

	static KgFbankPipe::KpOptions config_fbank(const nlohmann::json& jobjs)
	{
		auto opts = default_fbank();

		if (jobjs.contains("bank-norm") && jobjs["bank-norm"].is_boolean())
			opts.bankNorm = jobjs["bank-norm"].get<bool>();

		if (jobjs.contains("round-to-power2") && jobjs["round-to-power2"].is_boolean())
			opts.roundToPower2 = jobjs["round-to-power2"].get<bool>();

		if (jobjs.contains("energy-floor") && jobjs["energy-floor"].is_number())
			opts.energyFloor = jobjs["energy-floor"].get<double>();

		if (jobjs.contains("low-freq") && jobjs["low-freq"].is_number())
			opts.lowFreq = jobjs["low-freq"].get<double>();

		if (jobjs.contains("high-freq") && jobjs["high-freq"].is_number())
			opts.highFreq = jobjs["high-freq"].get<double>();

		if (jobjs.contains("num-banks") && jobjs["num-banks"].is_number_unsigned())
			opts.numBanks = jobjs["num-banks"].get<unsigned>();

		if (jobjs.contains("bank-type") && jobjs["bank-type"].is_string()) {
			auto type = jobjs["bank-type"].get<std::string>();
			opts.bankType = KgFbank::str2Type(type.c_str());
		}

		if (jobjs.contains("spec-type") && jobjs["spec-type"].is_string()) {
			auto type = jobjs["spec-type"].get<std::string>();
			opts.specType = KgSpectrum::str2Type(type.c_str());
		}

		if (jobjs.contains("spec-norm") && jobjs["spec-norm"].is_string()) {
			auto norm = jobjs["spec-norm"].get<std::string>();
			opts.specNorm = KgSpectrum::str2Norm(norm.c_str());
		}

		return opts;
	}

	static KgMfccPipe::KpOptions default_mfcc()
	{
		KgMfccPipe::KpOptions opts;
		opts.numCeps = 13;
		opts.cepsLifter = 22;

		return opts;
	}

	static KgMfccPipe::KpOptions config_mfcc(const nlohmann::json& jobjs)
	{
		auto opts = default_mfcc();

		if (jobjs.contains("ceps-lifter") && jobjs["ceps-lifter"].is_number())
			opts.cepsLifter = jobjs["ceps-lifter"].get<double>();

		if (jobjs.contains("num-ceps") && jobjs["num-ceps"].is_number_unsigned())
			opts.numCeps = jobjs["num-ceps"].get<unsigned>();

		auto fbankOpts = config_fbank(jobjs);
		memcpy(&opts, &fbankOpts, sizeof(fbankOpts));

		return opts;
	}

	static KvFeatPipeline* config_feat(const nlohmann::json& jobj, const KgPreprocess::KpOptions& opts)
	{
		KgMfccPipe::KpOptions mfccOpts = default_mfcc();
		bool makeMfcc(true);

		if (jobj.contains("feat") && jobj["feat"].is_object()) {
			auto feat = jobj["feat"];

			std::string type = "mfcc";
			if (feat.contains("type") && feat["type"].is_string()) 
				type = feat["type"].get<std::string>();

			makeMfcc = _strcmpi(type.c_str(), "mfcc") == 0;

			if (makeMfcc) mfccOpts = config_mfcc(jobj);
				
			auto fbankOpts = config_fbank(jobj);
			memcpy(&mfccOpts, &fbankOpts, sizeof(fbankOpts));
		}
		else {
			auto fbankOpts = default_fbank();
			memcpy(&mfccOpts, &fbankOpts, sizeof(fbankOpts));
		}

		memcpy(&mfccOpts, &opts, sizeof(opts)); // 复制KgPreprocess有关参数

		return makeMfcc ? (KvFeatPipeline*)(new KtFeatPipeProxy<KgMfccPipe>(mfccOpts))
			: (KvFeatPipeline*)(new KtFeatPipeProxy<KgFbankPipe>(mfccOpts));
	}

	static std::pair<KcVoicePicker*, KvFeatPipeline*> config(const nlohmann::json& jobj)
	{
		auto input = config_input(jobj);
		auto prep = config_prep(jobj, input->options().sampleRate);
		auto feat = config_feat(jobj, prep);

		return { input, feat };
	}
}


unsigned KgAsrFrontFrame::odim() const
{
	return ((KvFeatPipeline*)pipe_)->odim();
}
