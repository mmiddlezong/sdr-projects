#ifndef EXTRAPOLATION_H
#define EXTRAPOLATION_H

#include <vector>
#include <stdexcept>
#include <cmath>

using namespace std;

enum ExtrapolationMethod {
    linear,
    piecewise,
    quadratic,
    none,
    regression
};

float extrapolateNext(vector<float> &data, int index, const ExtrapolationMethod &method);

#endif // EXTRAPOLATION_H