#ifndef EXTRAPOLATION_H
#define EXTRAPOLATION_H

#include <vector>
#include <stdexcept>

using namespace std;

enum ExtrapolationMethod {
    linear,
    piecewise,
    quadratic,
    none
};

float extrapolateNext(vector<float> &data, int index, const ExtrapolationMethod &method);

#endif // EXTRAPOLATION_H