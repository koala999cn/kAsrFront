#include "KgAsrFrontFrame.h"
#include <fstream>
#include <list>
#include <string>
#include <assert.h>
#include "nlohmann/json.hpp"
#include "capture/KgWindowing.h"
#include "feature/KgPreprocess.h"
#include "feature/KgFbankPipe.h"
#include "feature/KgMfccPipe.h"
#include "feature/KgDelta.h"
#include "KvFeatPipeline.h"


namespace kPrivate
{
	class KgAsrFrontFrameImpl_
	{
	public:
		KgAsrFrontFrameImpl_(const nlohmann::json& jobj);

	private:
		static KcVoicePicker* configInput_(const nlohmann::json& jobj);
		static KgPreprocess::KpOptions defaultPrep_(double sampleRate);
		static KgPreprocess::KpOptions configPrep_(const nlohmann::json& jobj, double sampleRate);
		static KgFbankPipe::KpOptions defaultFbank_();
		static KgFbankPipe::KpOptions configFbank_(const nlohmann::json& jobjs);
		static KgMfccPipe::KpOptions defaultMfcc_();
		static KgMfccPipe::KpOptions configMfcc_(const nlohmann::json& jobjs);
		static KvFeatPipeline* configPipe_(const nlohmann::json& jobj, const KgPreprocess::KpOptions& opts);

	public:
		std::unique_ptr<KcVoicePicker> picker_;
		std::unique_ptr<KvFeatPipeline> pipe_;
		KgPreprocess::KpOptions prepOpts_;
	};
}


KgAsrFrontFrame::KgAsrFrontFrame(const char* jsonPath)
	: dptr_(nullptr), tracking_(false)
{
	std::ifstream ifs(jsonPath);
	nlohmann::json jobj;
	try {
		ifs >> jobj;
	}
	catch (...) {
		;
	}

	auto d = new kPrivate::KgAsrFrontFrameImpl_(jobj);
	dptr_ = d;

	if(d->pipe_)
		d->pipe_->setHandler([this](double* data) {
			feats_.push_back(std::vector<double>(data, data + odim()));	});
}


KgAsrFrontFrame::~KgAsrFrontFrame()
{
	stop();
	delete (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
}


bool KgAsrFrontFrame::run(std::function<void(const feat_matrix& feats)> h)
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;

	return d->picker_->run([h, d, this](KcVoicePicker::KeVoiceEvent e, const KcVoicePicker::KpEventData& data) {
		if (vhandler_ && !vhandler_(e, data))
			return false;

		if (e == KcVoicePicker::KeVoiceEvent::k_voice_picked) {
			assert(feats_.empty());
			d->pipe_->process(data.inbuf, data.frames);
			//d->pipe_->flush();
	
			KgDelta::KpOptions opts;
			opts.idim = odim();
			opts.order = 2;
			opts.window = 2;
			KgDelta delta(opts);
			auto feats = delta.compute(feats_);
			feats_.clear();

			h(feats);
		}

		return true;
		});
}


void KgAsrFrontFrame::stop()
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
	if(d && d->picker_)
		d->picker_->stop();
}


void KgAsrFrontFrame::process(const double* buf, unsigned frames) const
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
	if (d && d->pipe_)
		d->pipe_->process(buf, frames);
}



unsigned KgAsrFrontFrame::odim() const
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
	return d->pipe_->odim();
}


double KgAsrFrontFrame::sampleRate() const
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
	return d->prepOpts_.sampleRate;
}


double KgAsrFrontFrame::frameTime() const
{
	return double(frameSize()) / sampleRate();
}


unsigned KgAsrFrontFrame::frameSize() const
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
	return d->prepOpts_.frameSize;
}


double KgAsrFrontFrame::shiftTime() const
{
	return double(shiftSize()) / sampleRate();
}


unsigned KgAsrFrontFrame::shiftSize() const
{
	auto d = (kPrivate::KgAsrFrontFrameImpl_*)dptr_;
	return d->prepOpts_.frameShift;
}


namespace kPrivate
{
	KgAsrFrontFrameImpl_::KgAsrFrontFrameImpl_(const nlohmann::json& jobj)
	{
		picker_.reset(configInput_(jobj));
		if (picker_) {
			prepOpts_ = configPrep_(jobj, picker_->options().sampleRate);
			pipe_.reset(configPipe_(jobj, prepOpts_));
		}
	}

	KcVoicePicker* KgAsrFrontFrameImpl_::configInput_(const nlohmann::json& jobj)
	{
		KcVoicePicker::KpOptions opts;
		opts.deviceId = -1; // default input device
		opts.sampleRate = 16000;
		opts.vadMode = 0;
		opts.minVoiceDuration = 0.3;
		opts.maxWaitTime = 0.5;
		opts.aheadPaddingTime = 0.05;
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

	KgPreprocess::KpOptions KgAsrFrontFrameImpl_::defaultPrep_(double sampleRate)
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

	KgPreprocess::KpOptions KgAsrFrontFrameImpl_::configPrep_(const nlohmann::json& jobj, double sampleRate)
	{
		auto opts = defaultPrep_(sampleRate);

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

	KgFbankPipe::KpOptions KgAsrFrontFrameImpl_::defaultFbank_()
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

	KgFbankPipe::KpOptions KgAsrFrontFrameImpl_::configFbank_(const nlohmann::json& jobjs)
	{
		auto opts = defaultFbank_();

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

	KgMfccPipe::KpOptions KgAsrFrontFrameImpl_::defaultMfcc_()
	{
		KgMfccPipe::KpOptions opts;
		opts.numCeps = 13;
		opts.cepsLifter = 22;

		return opts;
	}

	KgMfccPipe::KpOptions KgAsrFrontFrameImpl_::configMfcc_(const nlohmann::json& jobjs)
	{
		auto opts = defaultMfcc_();

		if (jobjs.contains("ceps-lifter") && jobjs["ceps-lifter"].is_number())
			opts.cepsLifter = jobjs["ceps-lifter"].get<double>();

		if (jobjs.contains("num-ceps") && jobjs["num-ceps"].is_number_unsigned())
			opts.numCeps = jobjs["num-ceps"].get<unsigned>();

		auto fbankOpts = configFbank_(jobjs);
		memcpy(&opts, &fbankOpts, sizeof(fbankOpts));

		return opts;
	}

	KvFeatPipeline* KgAsrFrontFrameImpl_::configPipe_(const nlohmann::json& jobj, const KgPreprocess::KpOptions& opts)
	{
		KgMfccPipe::KpOptions mfccOpts;
		bool makeMfcc(true);

		if (jobj.contains("feat") && jobj["feat"].is_object()) {
			auto feat = jobj["feat"];

			std::string type = "mfcc";
			if (feat.contains("type") && feat["type"].is_string()) 
				type = feat["type"].get<std::string>();

			makeMfcc = _strcmpi(type.c_str(), "mfcc") == 0;

			if (makeMfcc) mfccOpts = configMfcc_(jobj);
	
		}
		else {
			mfccOpts = defaultMfcc_();
			auto fbankOpts = defaultFbank_();
			memcpy(&mfccOpts, &fbankOpts, sizeof(fbankOpts));
		}

		memcpy(&mfccOpts, &opts, sizeof(opts)); // 复制KgPreprocess有关参数

		return makeMfcc ? (KvFeatPipeline*)(new KtFeatPipeProxy<KgMfccPipe>(mfccOpts))
			: (KvFeatPipeline*)(new KtFeatPipeProxy<KgFbankPipe>(mfccOpts));
	}
}
