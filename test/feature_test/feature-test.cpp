#include <fstream>
#include <string>
#include <vector>
#include "feature/KgPreprocess.h"
#include "feature/KgFbank.h"


void test_failed();
void fft_test(const std::vector<double>& wav);
void spectrum_test(const std::vector<double>& wav);
void spectrogram_test(const std::vector<double>& wav);
void fbank_test(const std::vector<double>& wav);
void mfcc_test(const std::vector<double>& wav);

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


int main()
{
    auto wav = read_test_wav();
    if (wav.empty())
        return 1;

    mfcc_test(wav);
    fbank_test(wav);
    fft_test(wav);
    spectrum_test(wav);
    spectrogram_test(wav);

    printf("  :) All Passed.\n");

    return 0;
}
