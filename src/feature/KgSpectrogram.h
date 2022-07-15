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

	using spcetrum_handler = std::function<bool(double* spec)>;

	KgSpectrogram(const KpOptions& opts, spcetrum_handler h);

	~KgSpectrogram();

	void process(const double* buf, unsigned len) const;

	void flush() const;

	// ֻ�����ά�ȣ�����ά�����û��ṩ
	unsigned dim() const;

private:

	// ����Ƶ�����ͺ͹�һ��ģʽ����������ֵ
	double fixEnergy_(double energy) const;

private:
	unsigned fftFramingSize_; // ������fft��frame��С��roundToPower2Ϊ��ʱ���������frameSize��Ϊ��ʱ����ceilPower2(frameSize)
	double energyFloor_;
	std::unique_ptr<KgPreprocess> prep_;
	void* dptr_;
};
