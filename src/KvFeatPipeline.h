#pragma once


class KvFeatPipeline
{
public:

	virtual ~KvFeatPipeline() {}

	virtual void setHandler(std::function<void(double*)>) = 0;

	virtual void process(const double* buf, unsigned len) const = 0;

	virtual void flush() const = 0;

	virtual unsigned odim() const = 0;
};


template<typename PIPE>
class KtFeatPipeProxy : public KvFeatPipeline
{
public:

	KtFeatPipeProxy(typename const PIPE::KpOptions& opts)
		: pipe_(opts) {}

	void setHandler(std::function<void(double*)> h) override {
		return pipe_.setHandler(h);
	}

	void process(const double* buf, unsigned len) const override {
		pipe_.process(buf, len);
	}

	void flush() const override {
		pipe_.flush();
	}

	unsigned odim() const override {
		return pipe_.odim();
	}

private:
	PIPE pipe_;
};