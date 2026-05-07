#include "Utility.h"

#include <cmath>

bool approximately_equal(double a, double b) {
    return std::abs(a - b) < 1e-9;
}