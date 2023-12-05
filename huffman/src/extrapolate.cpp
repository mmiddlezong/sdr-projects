#include "extrapolate.h"

int maxLookback = 3; // Maximum number of data points to consider for extrapolation

float extrapolateNext(vector<float> &data, int index, const ExtrapolationMethod &method) {
    if (method == none || index < 1) {
        return data[0];
    } else if (method == piecewise || index < 2) {
        return data[index - 1];
    } else if (method == linear || index < 3) {
        return 2 * data[index - 1] - data[index - 2];
    } else if (method == quadratic) {
        return data[index - 3] - 3 * data[index - 2] + 3 * data[index - 1];
    } else if (method == regression) {
        // Perform linear regression
        int n = min(index - 1, maxLookback);
        float sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
        for (int i = index - n; i < index; i++) {
            sumX += i;
            sumY += data[i];
            sumXY += i * data[i];
            sumX2 += i * i;
        }
        float slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
        float intercept = (sumY - slope * sumX) / n;
        float result = slope * index + intercept;
        if (isnan(result)) {
            return 2 * data[index - 1] - data[index - 2]; // Default to linear extrapolation
        }
        return result;
    } else {
        throw runtime_error("Unknown extrapolation method.");
    }
}