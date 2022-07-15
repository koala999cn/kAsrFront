#include <fstream>
#include <string>
#include <vector>
#include "base/KgRdft.h"
#include "base/KtuMath.h"
#include "base/KtuBitwise.h"
#include "test-util.h"


void test_kaldi_fft(const std::vector<double>& fft, const char* kaldiRef)
{
    auto kaldi = load_matrix(kaldiRef);
    if (kaldi.size() != 1 || kaldi[0].size() != fft.size()) {
        printf("%s missing or corupt.", kaldiRef);
        test_failed();
    }
    // 与kaldi偏差较大，不进行equal-test，搜集偏差情况
    dump_bias(fft, kaldi.front());
}

void fft_test(const std::vector<double>& wav)
{
    printf(" fft test...  \n");

    std::vector<double> data(wav);
    KgRdft rdft(wav.size());

    rdft.forward(data.data());

    printf("  test with fftw...  ");
    auto fftw = load_matrix("../data/fft-fftw3.txt"); // 数据布局：re[0], im[0], re[1], im[1], ...
    if (fftw.size() != rdft.sizeF() * 2) {
        printf("fft-fftw2.txt missing or corupt.");
        test_failed();
    }
    for (unsigned i = 0; i < rdft.sizeF(); i++) {
        auto spec = rdft.unpack(data.data(), i);
        if (!KtuMath<double>::almostEqualRel(spec.first, fftw[2 * i].front()) ||
            !KtuMath<double>::almostEqualRel(spec.second, fftw[2 * i + 1].front())) {
            printf("spec[%d] mismatch: <%f, %f> vs <%f, %f>", 
                i, spec.first, spec.second, fftw[2 * i].front(), fftw[2 * i + 1].front());
            test_failed();
        }
    }
    printf(":)passed.\n");


    printf("  test with praat...  ");
    auto praat = load_matrix("../data/fft-praat.txt"); // 数据布局：re[0], re[0], ..., im[0], im[1], ...
    if (praat.size() != rdft.sizeF() * 2) {
        printf("fft-praat.txt missing or corupt.");
        test_failed();
    }
    for (unsigned i = 0; i < rdft.sizeF(); i++) {
        auto spec = rdft.unpack(data.data(), i);
        // 数据相差sampleRate倍数
        if (!KtuMath<double>::almostEqualRel(spec.first / 16000, praat[i].front()) ||
            !KtuMath<double>::almostEqualRel(spec.second / 16000, praat[i + rdft.sizeF()].front())) {
            printf("spec[%d] mismatch: <%f, %f> vs <%f, %f>",
                i, spec.first/16000, spec.second/16000, praat[i].front(), praat[i + rdft.sizeF()].front());
            test_failed();
        }
    }
    printf(":)passed.\n");

    printf("  test with kaldi plain...  ");
    {
        data = wav;
        KgRdft rdft(data.size(), true); // 按kaldi进行数据布局
        rdft.forward(data.data());
        // kaldi设定的wav为int16范围，在此处乘回来
        KtuMath<double>::scale(data.data(), data.size(), std::numeric_limits<std::int16_t>::max());
    }
    test_kaldi_fft(data, "../data/fft-kaldi-plain.txt");

    printf("  test with kaldi round-to-power2...  ");
    {
        data = wav;
        auto n = KtuBitwise<int>::ceilPower2(wav.size());
        data.resize(n);
        std::fill(data.begin() + wav.size(), data.end(), 0);
        KgRdft rdft(data.size(), true); // 按kaldi进行数据布局
        rdft.forward(data.data());
        // kaldi设定的wav为int16范围，在此处乘回来
        KtuMath<double>::scale(data.data(), data.size(), std::numeric_limits<std::int16_t>::max());
    }
    test_kaldi_fft(data, "../data/fft-kaldi-round.txt");
}
