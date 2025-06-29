#include "random.h"

#include <cmath>

namespace diesel {
  const double DIESEL_LCG_MODULUS = 2.328306436538696e-10;
  const int DIESEL_LCG_MULTIPLIER = 1664525;
  const int DIESEL_LCG_INCREMENT = 1013904223;

  DieselLCG::DieselLCG() {
    // Default value of Diesel's LCG is generated from the dsl::math_library::cryptrand function which:
    // - MD5 hashes "crypt"
    // - Updates the hash with another 32bit global LCG
    // - Updates the hash with the 64bit CPU time stamp (rdtsc instruction)
    // - Updates the hash with the 64bit QueryPerformanceCounter result
    // - Updates the hash with the 32bit floating point game timer value
    // - Updates the hash with the 32bit unsigned number of how many times cryptrand has been called in the executable's lifetime
    // - Sets the LCG state to the first 4 bytes of the stringified MD5 digest interpreted as a 32bit integer (this is as of PDTH, this step might have changed in newer versions)
    this->_state = 0;
  }

  // dsl::math_library::script_random
  double DieselLCG::random() {
    this->_state = DIESEL_LCG_MULTIPLIER * this->_state + DIESEL_LCG_INCREMENT;
    return this->_state * DIESEL_LCG_MODULUS;
  }
  double DieselLCG::random(double max) {
    this->_state = DIESEL_LCG_MULTIPLIER * this->_state + DIESEL_LCG_INCREMENT;
    return floor(this->_state * DIESEL_LCG_MODULUS * max + 1.0);
  }
  double DieselLCG::random(double min, double max) {
    this->_state = DIESEL_LCG_MULTIPLIER * this->_state + DIESEL_LCG_INCREMENT;
    return floor(this->_state * DIESEL_LCG_MODULUS * (max - min + 1.0) + min);
  }
}
