#include <fstream>
#include <string>
#include <vector>
#include "base/KtuMath.h"
#include "base/KuStrUtil.h"
#include "test-util.h"


void test_failed()
{
    printf("  :( failed.");
    return quick_exit(1);
}


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
            if(KuStrUtil::toValue(tok.c_str(), val))
                row.push_back(val);
        }

        if(!row.empty())
            mat.push_back(row);
    }

    return mat;
}


vectord get_column(const matrixd& m, int col)
{
    vectord v;
    v.reserve(m.size());
    for (auto& i : m)
        v.push_back(i[col]);

    return v;
}


void equal_test(const vectord& v1, const vectord& v2, double refTol)
{
    if (v1.size() != v2.size()) {
        printf("data size mismatch: %d vs %d\n", int(v1.size()), int(v2.size()));
        test_failed();
    }

    for (unsigned i = 0; i < v1.size(); i++) {
        if (!KtuMath<double>::almostEqualRel(v1[i], v2[i], refTol)) {
            printf("value mismatch at [%d]: %f vs %f\n", i, v1[i], v2[i]);
            test_failed();
        }
    }
}


void equal_test(const matrixd& m1, const matrixd& m2, double refTol)
{
    if (m1.size() != m2.size()) {
        printf("row size mismatch: %d vs %d\n", int(m1.size()), int(m2.size()));
        test_failed();
    }

    for (unsigned i = 0; i < m1.size(); i++) {
        if (m1[i].size() != m2[i].size()) {
            printf("column size mismatch at row %d: %d vs %d\n", int(m1[i].size()), int(m2[i].size()), i);
            test_failed();
        }

        for (unsigned j = 0; j < m1[i].size(); j++) {
            if (!KtuMath<double>::almostEqualRel(m1[i][j], m2[i][j], refTol)) {
                printf("value mismatch at [%d][%d]: %f vs %f\n", i, j, m1[i][j], m2[i][j]);
                test_failed();
            }
        }
    }
}


void dump_bias(const vectord& v1, const vectord& v2)
{
    double maxBias(0), meanBias(0);
    double maxVal0(0), maxVal1(0);
    unsigned maxIdx(0);
    for (unsigned i = 0; i < v1.size(); i++) {
        double bias = std::abs((v2[i] - v1[i]) / v2[i]);
        meanBias += bias;
        if (bias > maxBias) {
            maxBias = bias;
            maxVal0 = v1[i];
            maxVal1 = v2[i];
            maxIdx = i;
        }
    }
    meanBias /= v1.size();
    printf("mean-bais: %f, max-bais: %f at index %d (%f vs %f)\n", meanBias, maxBias, maxIdx, maxVal0, maxVal1);
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

    printf("mean-bais: %f, max-bais: %f at r=%d, c=%d (%f vs %f)\n", meanBias, maxBias, maxr,maxc, maxVal0, maxVal1);
}