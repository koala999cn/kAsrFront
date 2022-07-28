#pragma once
#include "KgPreprocess.h"
#include "KtPipeline.h"
#include <memory>
#include <functional>


template<typename... OP>
class KtFeatPipeline
{
public:

	KtFeatPipeline(const KgPreprocess::KpOptions& opts, OP&&... ops) 
	    : pipeline_(std::forward<OP>(ops)...) {
		prep_ = std::make_unique<KgPreprocess>(opts);
		assert(pipeline_.idim() == prep_->odim());

		// ����ȱʡ����������handler
		prep_->setHandler([this](double* frame, double energy) {
			std::vector<double> out(odim());
			pipeline_.process(frame, out.data());
			if (prep_->options().useEnergy != KgPreprocess::k_use_energy_none)
				out[0] = energy; 
			handler_(out.data());
			});
	}

	void setHandler(std::function<void(double* out)> h) {
		handler_ = h;
	}

	void process(const double* buf, unsigned len) const {
		prep_->process(buf, len);
	}

	void flush() const {
		prep_->flush();
	}

	// ֻ�����ά�ȣ�����ά�����û��ṩ
	unsigned odim() const {
		return pipeline_.odim();
	}

protected:
	std::unique_ptr<KgPreprocess> prep_;
	KtPipeline<OP...> pipeline_;
	std::function<void(double*)> handler_;
};

/*
class KgSpectrogram
{
public:

	struct KpOptions : public KgPreprocess::KpOptions
	{
		int type; // Ƶ������, �μ�KgSpectrum::KeType
		int norm; // ��һ��ģʽ���μ�KgSpectrum::KeNormMode
		bool roundToPower2;
		double energyFloor; // ���ź�energyС�ڸ�ֵʱ���ø�ֵ����ź�energyֵ����norm�߶����
	};

	
	KgSpectrogram(const KpOptions& opts);

	~KgSpectrogram();

	void setHandler(std::function<void(double* spec)> h);

	void process(const double* buf, unsigned len) const;

	void flush() const;

	// ֻ�����ά�ȣ�����ά�����û��ṩ
	unsigned odim() const;

private:

	void processOneFrame_(double* in, double* out) const;

	// ����Ƶ�����ͺ͹�һ��ģʽ����������ֵ
	double fixEnergy_(double energy) const;

private:
	double energyFloor_;
	std::unique_ptr<KgPreprocess> prep_;
	void* dptr_;
	std::function<void(double* spec)> handler_;
};
*/