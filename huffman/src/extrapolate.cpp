#include "extrapolate.h"

float extrapolateNext(vector<float> &data, int index, const ExtrapolationMethod &method) {
    if (method == none || index < 1) {
        return data[0];
    } else if (method == piecewise || index < 2) {
        return data[index - 1];
    } else if (method == linear || index < 3) {
        return 2 * data[index - 1] - data[index - 2];
    } else if (method == quadratic) {
        return data[index - 3] - 3 * data[index - 2] + 3 * data[index - 1];
    } else {
        throw runtime_error("Unknown extrapolation method.");
    }
}