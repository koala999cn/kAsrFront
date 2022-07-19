#pragma once
#include "KgPreprocess.h"
#include <memory>


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

	using spcetrum_handler = std::function<bool(double* spec)>;

	void process(const double* buf, unsigned len, spcetrum_handler sh) const;

	void flush(spcetrum_handler sh) const;

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
};
