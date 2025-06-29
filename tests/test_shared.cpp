#include "test_shared.h"

#include <cmath>

bool test_double_nearly_equal(double a, double b) {
  if (abs(a - b) < 0.000001) return true;
  return false;
}