#include "KgAsrFrontFrame.h"
#include "base/KuStrUtil.h"
#include <fstream>


using matrixd = std::vector<std::vector<double>>;

matrixd load_matrix(const char* path)
{
    matrixd mat;

    std::ifstream ifs(path);
    while (ifs) {
        std::string line;

        std::getline(ifs, line);
        auto tokens = KuStrUtil::split(line, "\t ,");
        if (tokens.empty())
            continue; // empty line

        std::vector<double> row;
        row.reserve(tokens.size());
        for (auto& tok : tokens) {
            double val;
            if (KuStrUtil::toValue(tok.c_str(), val))
                row.push_back(val);
        }

        if (!row.empty())
            mat.push_back(row);
    }

    return mat;
}


void dump_bias(const matrixd& m1, const matrixd& m2)
{
    double maxBias(0), meanBias(0);
    double maxVal0(0), maxVal1(0);
    unsigned maxr(0), maxc(0);
    for (unsigned r = 0; r < m1.size(); r++) {
        auto& v1 = m1[r];
        auto& v2 = m2[r];
        for (unsigned i = 0; i < v1.size(); i++) {
            double bias = std::abs(v2[i] - v1[i]);
            if (v2[i] != 0) bias /= std::abs(v2[i]);
            meanBias += bias;
            if (bias > maxBias) {
                maxBias = bias;
                maxVal0 = v1[i];
                maxVal1 = v2[i];
                maxr = r, maxc = i;
            }
        }
    }
    meanBias /= m1.size() * m1[0].size();

    printf("mean-bais: %f, max-bais: %f at r=%d, c=%d (%f vs %f)\n", 
        meanBias, maxBias, maxr, maxc, maxVal0, maxVal1);
}


int main(int argc, char const* argv[])
{
	KgAsrFrontFrame front("front-end.json");

    printf("load wav data... ");
    auto m = load_matrix("../data/test.txt");
    std::vector<double> wav; wav.reserve(m.size());
    for (auto& i : m)
        wav.push_back(i.front());

    printf("%d samples\n", int(wav.size()));

    front.process(wav.data(), wav.size());
    
    m = load_matrix("../data/mfcc.txt");
    dump_bias(front.features(), m);

    return 0;
}
