#include "KgMfcc.h"
#include <vector>
#include <memory>
#include "KgDct.h"
#include "KuCepsUtil.h"
#include "KtuMath.h"


namespace kPrivate
{
	struct KpMfccPrivate
	{
		std::unique_ptr<KgDct> dct;
		std::vector<double> lifterWeigths;
	};
}


KgMfcc::KgMfcc(unsigned idim, const KpOptions& opts)
{
	idim_ = idim;
	numCeps_ = opts.numCeps;
	auto d = new kPrivate::KpMfccPrivate;
	d->dct = std::make_unique<KgDct>(idim, numCeps_, true); // TODO: ��������norm
	dptr_ = d;

	if (opts.cepsLifter != 0.0) {
		d->lifterWeigths.resize(numCeps_);
		KuCepsUtil::makeLifter_Juang(numCeps_, opts.cepsLifter, d->lifterWeigths.data());
	}
}


KgMfcc::~KgMfcc()
{
	delete (kPrivate::KpMfccPrivate*)dptr_;
}


unsigned KgMfcc::idim() const
{
	return idim_;
}


unsigned KgMfcc::odim() const
{
	return numCeps_;
}


void KgMfcc::process(const double* in, double* out)
{
	auto d = (kPrivate::KpMfccPrivate*)dptr_;
	d->dct->forward(in, out);
	if (!d->lifterWeigths.empty()) {
		assert(d->lifterWeigths.size() == odim());
		KtuMath<double>::mul(d->lifterWeigths.data(), out, out, odim());
	}
}