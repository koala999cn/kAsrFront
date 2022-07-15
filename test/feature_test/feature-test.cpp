#include <fstream>
#include <string>
#include <vector>
#include "feature/KgPreprocess.h"
#include "feature/KgFbank.h"


void test_failed();
void fft_test(const std::vector<double>& wav);
void spectrum_test(const std::vector<double>& wav);

static std::vector<double> read_test_wav()
{
    std::vector<double> wav;

    printf("loading test data from '../data/test.txt'...  ");

    std::ifstream ifs;
    ifs.open("../data/test.txt");
    if (!ifs) {
        printf("file missing");
        test_failed();
    }
    else {
        while (ifs) {
            std::string str;
            ifs >> str;
            if (!str.empty())
                wav.push_back(std::atof(str.c_str()));
        }
    }

    printf("%d samples\n", int(wav.size()));

    return wav;
}


#if 0
static void test_fbank(const std::vector<double>& wav)
{
    KgPreprocess::KpOptions prepOpts;
    make_plain(prepOpts);

    KgRdft fft(prepOpts.frameSize);

    std::vector<std::vector<double>> res;
    KgPreprocess prep(prepOpts, [&fft, &res](double* frame, double energy) {
        std::vector<double> buf(fft.sizeT());
        std::copy(frame, frame + fft.sizeT(), buf.begin());
        // fft.forward(buf.data());
         //fft.powerSpectrum(buf.data());
        res.push_back(buf);
        return true;
        });

    prep.process(wav.data(), wav.size());
    prep.flush();

    auto ref = load_matrix("../data/fft-plain.scp");

    testEqual(res, ref);
}
#endif

int main()
{
    auto wav = read_test_wav();
    if (wav.empty())
        return 1;

    fft_test(wav);
    spectrum_test(wav);
    
    printf("  :) All Passed.\n");

    return 0;
}
