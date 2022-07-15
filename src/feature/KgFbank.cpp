#include "KgFbank.h"
#include <assert.h>
#include <functional>
#include "KtSampling.h"
#include "KuFreqUnit.h"
#include "KtuMath.h"


KgFbank::KgFbank(double sampleRate, double df, const KpOptions& opts)
    : opts_(opts)
{
    if (opts_.highFreq <= 0)
        opts_.highFreq = sampleRate / 2; // 取奈奎斯特频率

    // 边界检测
    assert(opts_.lowFreq >= 0 && opts_.highFreq > opts_.lowFreq);
    assert(opts_.type >= k_linear && opts_.type <= k_erb);

    // Hz尺度的采样参数
    KtSampling<double> sampHz;
    sampHz.reset(0, sampleRate / 2, df, 0.5);
    isize_ = static_cast<unsigned>(sampHz.size()); 

    // 目标(type_)尺度的采样参数
    auto lowScale = fromHertz_(opts_.lowFreq);
    auto highScale = fromHertz_(opts_.highFreq);
    KtSampling<double> sampTy;
    sampTy.resetn(opts_.numBins + 1, lowScale, highScale, 0); // 在目标尺度上均匀划分各bin，相邻的bin有1/2重叠

    firstIdx_.resize(opts_.numBins);
    fc_.resize(opts_.numBins);
    weights_.resize(opts_.numBins);
    for (unsigned bin = 0; bin < opts_.numBins; bin++) {
        // 计算目标尺度上的bin参数（左边频率，右边频率，中心频率）
        auto fl = sampTy.indexToX(bin);
        auto fc = sampTy.indexToX(bin + 1);
        auto fr = sampTy.indexToX(bin + 2);

        // 换算到Hertz尺度上保存
        auto flhz = toHertz_(fl);
        auto frhz = toHertz_(fr);
        auto idx = sampHz.rangeToIndex(flhz, frhz);
        if (idx.first < 0) idx.first = 0;
        firstIdx_[bin] = idx.first;
        fc_[bin] = toHertz_(fc);

        if (idx.first < idx.second) {
            // 计算当前bin的权值数组（目标尺度上的三角滤波）
            auto factor = 1 / (frhz - flhz);
            weights_[bin].resize(idx.second - idx.first, 0);
            for (long i = idx.first; i < idx.second; i++)
                weights_[bin][i - idx.first] = factor *
                calcFilterWeight_(fl, fr, fromHertz_(sampHz.indexToX(i)));
        }
        else {
            weights_[bin].clear();
        }
    }
}


std::pair<unsigned, unsigned> KgFbank::dim() const
{
    return { isize_, opts_.numBins };
}


void KgFbank::process(const double* in, double* out)
{
    for (unsigned i = 0; i < opts_.numBins; i++) {
        unsigned N = isize_ > firstIdx_[i] ? 
            std::min(unsigned(weights_[i].size()), isize_ - firstIdx_[i]) : 0;

        // 若N=0, dot返回0.0
        out[i] = KtuMath<double>::dot(in + firstIdx_[i], weights_[i].data(), N);
    }
}


double KgFbank::toHertz_(double scale)
{
    static std::function<double(double)> cvt[] = {
        [](double f) { return f; }, // k_linear
        [](double f) { return exp(f) - 1; }, // k_log
        [](double f) { return KuFreqUnit::melToHertz(f); }, // k_mel
        [](double f) { return KuFreqUnit::barkToHertz(f); }, // k_bark
        [](double f) { return KuFreqUnit::camToHertz(f); }  // k_erb
    };

    return cvt[opts_.type](scale);
}


double KgFbank::fromHertz_(double hz)
{
    static std::function<double(double)> cvt[] = {
        [](double hz) { return hz; }, // k_linear
        [](double hz) { return log(hz + 1); }, // k_log
        [](double hz) { return KuFreqUnit::hertzToMel(hz); }, // k_mel
        [](double hz) { return KuFreqUnit::hertzToBark(hz); }, // k_bark
        [](double hz) { return KuFreqUnit::hertzToCam(hz); }  // k_erb
    };

    return cvt[opts_.type](hz);
}


double KgFbank::calcFilterWeight_(double low, double high, double f)
{
    auto half = 0.5 * (high - low);
    return f <= low + half ? (f - low) / half : (high - f) / half;
}