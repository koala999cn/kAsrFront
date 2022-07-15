#pragma once
#include <vector>

using vectord = std::vector<double>;
using matrixd = std::vector<vectord>;

void test_failed();

matrixd load_matrix(const char* path);

void equal_test(const vectord& v1, const vectord& v2, double refTol = 0.001);

void equal_test(const matrixd& m1, const matrixd& m2, double refTol = 0.001);

void dump_bias(const vectord& v1, const vectord& v2);

vectord get_column(const matrixd& m, int col);